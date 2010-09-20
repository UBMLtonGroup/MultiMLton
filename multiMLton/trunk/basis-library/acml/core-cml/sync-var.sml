structure SyncVar : SYNC_VAR_EXTRA =
   struct
      structure Assert = LocalAssert(val assert = false)
      structure Debug = LocalDebug(val debug = false)

      structure Q = ImpQueue
      structure S = Scheduler
      structure E = Event
      structure L = Lock
      structure B = Basic

      fun debug msg = Debug.sayDebug ([S.atomicMsg, S.tidMsg], msg)
      fun debug' msg = debug (fn () => msg^" : "
                                    ^Int.toString(B.processorNumber()))

      datatype trans_id = datatype TransID.trans_id
      datatype trans_id_state = datatype TransID.trans_id_state


      (* the underlying representation of both ivars and mvars is the same. *)
      datatype 'a cell =
         CELL of {prio  : int ref,
                  readQ : (trans_id * 'a S.thread * int * (unit -> unit)) Q.t,
                  value : 'a option ref,
                  lock  : L.cmlLock}

      type 'a ivar = 'a cell
      type 'a mvar = 'a cell

      exception Put

      fun newCell () = CELL {prio = ref 0, readQ = Q.new(), value = ref NONE,
        lock = L.initCmlLock ()}

      (* sameCell : ('a cell * 'a cell) -> bool *)
      fun sameCell (CELL {prio = prio1, ...}, CELL {prio = prio2, ...}) =
         prio1 = prio2

      (* bump a priority value by one, returning the old value *)
      fun bumpPriority (p as ref n) = (p := n+1; n)

      (* functions to clean channel input and output queues *)
      local
         fun cleaner (TXID {txst,cas} , _, _, _) =
            case !txst of SYNCHED => true | _ => false
      in
         fun cleanAndDeque q =
            Q.dequeLazyClean (q, cleaner)
         fun enqueAndClean (q, item) =
           (Q.cleanSuffix (q,cleaner);
            Q.enque (q, item))
      end

      fun pN () : int  = B.processorNumber ()

      (* When a thread is resumed after being blocked on an iGet or mGet operation,
       * there may be other threads also blocked on the variable.  This function
       * is used to propagate the message to all of the threads that are blocked
       * on the variable (or until one of them takes the value in the mvar case).
       * It must be called from an atomic region; when the readQ is finally empty,
       * we leave the atomic region.  We must use "cleanAndDeque" to get items
       * from the readQ in the unlikely event that a single thread executes a
       * choice of multiple gets on the same variable.
       *)
      fun relayMsg (readQ, msg, lock) =
       let
         val readyList = ref []
         fun tryLp () =
              (case (cleanAndDeque readQ) of
                  SOME (TXID {txst, cas}, t, procNum, doSwap) =>
                    let fun matchLp () =
                      (case cas(txst, WAITING, SYNCHED) of
                           WAITING => (* We got it *)
                              let
                                val _ = readyList := ((S.prepVal (t,msg), procNum)::(!readyList))
                                val _ = doSwap ()
                              in
                                tryLp ()
                              end
                         | CLAIMED => matchLp ()
                         | SYNCHED => tryLp ())
                      (* matchLp ends *)
                    in
                      case !txst of
                           SYNCHED => tryLp ()
                         | _ => matchLp ()
                    end
                    (* case SOME ends *)
                  | NONE =>
                      let
                        val rdyLst = !readyList
                        val _ = List.map (fn (rt,procNum) => S.readyOnProc (rt, procNum)) rdyLst
                      in
                        L.releaseCmlLock lock (S.tidNum())
                      end
              ) (* tryLp ends *)
       in
         tryLp ()
       end


      (** G-variables **)
      (* Generalized synchronized variables,
       * to factor out the common operations.
       *)

      fun gPut (name, CELL {prio, readQ, value, lock}, x) =
         let
            val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name])
            val () = debug' ( concat [name, "(1)"]) (* NonAtomic *)
            val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(1)"])
            val () = S.atomicBegin()
           val () = L.getCmlLock lock S.tidNum
            val () = debug' ( concat [name, "(2)"]) (* Atomic 1 *)
            val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(2)"], SOME 1)
            val () =
               case !value of
                  NONE =>
                     let
                        val () = debug' ( concat [name, "(3.1.1)"]) (* Atomic 1 *)
                        val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(3.1.1)"], SOME 1)
                        val () = value := SOME x
                        val () = prio := 1
                        (* Implicitly releases lock *)
                       val () = relayMsg (readQ, x, lock)
                       val () = ThreadID.mark (B.getCurThreadId ())
                       val () = S.atomicEnd ()
                        val () = debug' ( concat [name, "(3.1.2)"]) (* NonAtomic *)
                        val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(3.1.2)"])
                     in
                        ()
                     end
                | SOME _ =>
                     let
                        val () = debug' ( concat [name, "(3.2.1)"]) (* Atomic 1 *)
                        val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(3.2.1)"], SOME 1)
                       val () = L.releaseCmlLock lock (S.tidNum ())
                        val () = S.atomicEnd ()
                        val () = debug' ( concat [name, "(3.2.2)"]) (* NonAtomic *)
                        val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(3.2.2)"])
                     in
                        raise Put
                     end
            val () = debug' ( concat [name, "(4)"]) (* NonAtomic *)
            val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(4)"])
         in
            ()
         end

      (* Swap the current contents of the cell with a new value;
       * it is guaranteed to be atomic.
       *)
      fun gSwap (name, doSwap, CELL {prio, readQ, value, lock}) =
         let
            val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, ""])
            val () = debug' ( concat [name, "(1)"]) (* NonAtomic *)
            val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(1)"])
            val () = S.atomicBegin()
           val curProcNum = pN ()
           val () = L.getCmlLock lock S.tidNum
            val () = debug' ( concat [name, "(2)"]) (* Atomic 1 *)
            val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(2)"], SOME 1)
            val msg =
               case !value of
                  NONE =>
                     let
                        val () = debug' ( concat [name, "(3.2.1)"]) (* Atomic 1 *)
                        val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(3.2.1)"], SOME 1)
                        val msg =
                           S.atomicSwitchToNext
                           (fn rt => (enqueAndClean (readQ, (TransID.mkTxId (),
                           rt, curProcNum , fn () => doSwap value));
                                      L.releaseCmlLock lock (S.tidNum())))
                        val () = debug' ( concat [name, "(3.2.3)"]) (* NonAtomic *)
                        val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(3.2.3)"])
                     in
                        msg
                     end
                | SOME x =>
                     let
                        val () = debug' ( concat [name, "(3.2.1)"]) (* Atomic 1 *)
                        val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(3.2.1)"], SOME 1)
                        val () = prio := 1
                        val () = doSwap value
                        val () = L.releaseCmlLock lock (S.tidNum())
                        val () = ThreadID.mark (B.getCurThreadId ())
                        val () = S.atomicEnd ()
                        val () = debug' ( concat [name, "(3.2.2)"]) (* NonAtomic *)
                        val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(3.2.2)"])
                     in
                        x
                     end
            val () = debug' ( concat [name, "(4)"]) (* NonAtomic *)
            val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(4)"])
         in
            msg
         end

      fun gSwapEvt (name, doSwap, CELL{prio, readQ, value, lock}) =
         let
            fun doitFn (_) =
               let
                  val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, ".doitFn"], NONE)
                  val () = debug' ( concat [name, "(3.2.1)"]) (* Atomic 1 *)
                  val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(3.2.1)"], SOME 1)
                  val () = L.getCmlLock lock S.tidNum
                  val x =
                    case !value of
                         NONE => (L.releaseCmlLock lock (S.tidNum()); NONE)
                       | SOME x => let
                                      val () = prio := 1
                                      val () = doSwap value
                                      val () = L.releaseCmlLock lock (S.tidNum())
                                      val () = S.atomicEnd ()
                                   in
                                     SOME x
                                   end
                  val () = debug' ( concat [name, "(3.2.2)"]) (* NonAtomic/Atomic 1 *)
                  val () = case x of
                                NONE => Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(3.2.2)"], SOME 1)
                              | SOME _ => Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(3.2.2)"])
               in
                  x
               end
            fun blockFn {transId as TXID {txst = myTxst, cas = myCas}, cleanUp, next, ...} =
               let
                  val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, ".blockFn"], NONE)
                  val () = debug' ( concat [name, "(3.2.1)"]) (* Atomic 1 *)
                  val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(3.2.1)"], SOME 1)
                  val curProcNum = pN ()
                  val () = L.getCmlLock lock S.tidNum
                  val msg =
                   case !value of
                        NONE => let
                                  val () = debug' ( concat [name, "(3.2.2).tryLp.NONE"]) (* Atomic 1 *)
                                  val msg = S.atomicSwitch
                                            (fn rt =>
                                              (enqueAndClean (readQ, (transId,
                                              rt, curProcNum, fn () => doSwap value))
                                              ; L.releaseCmlLock lock (S.tidNum ())
                                              ; next ()))
                                in
                                  msg
                                end
                      | SOME x =>
                          let
                            val () = debug' ( concat [name, "(3.2.2).tryLp.SOME"]) (* Atomic 1 *)
                            val msg =
                              let
                                fun matchLp () =
                                      (case myCas(myTxst, WAITING, CLAIMED) of
                                          (* try to claim the matching event *)
                                        WAITING => (prio := 1
                                                  ; myTxst := SYNCHED
                                                  ; L.releaseCmlLock lock (S.tidNum())
                                                  ; S.atomicEnd ()
                                                  ; x)
                                          (* In timeEvt *)
                                        | CLAIMED => matchLp ()
                                        | SYNCHED =>
                                                (L.releaseCmlLock lock (S.tidNum());
                                                S.atomicSwitchToNext (fn _ => ())))
                              in
                                matchLp ()
                              end
                          in
                            msg
                          end

                  val () = debug' ( concat [name, "(3.2.3)"]) (* NonAtomic *)
                  val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(3.2.3)"])
               in
                  msg
               end
            fun pollFn () =
               let
                  val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, ".pollFn"], NONE)
                  val () = debug' ( concat [name, "(2)"]) (* Atomic 1 *)
                  val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(2)"], SOME 1)
               in
                  case !value of
                     NONE => E.blocked blockFn
                   | SOME _ => E.enabled {prio = bumpPriority prio,
                                          doitFn = doitFn}
               end
         in
            E.bevt pollFn
         end

      fun gSwapPoll (name, doSwap, CELL{prio, value, lock, ...}) =
         let
            val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, ""])
            val () = debug' ( concat [name, "(1)"]) (* NonAtomic *)
            val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(1)"])
            val () = S.atomicBegin()
           val () = L.getCmlLock lock S.tidNum
            val () = debug' ( concat [name, "(2)"]) (* Atomic 1 *)
            val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(2)"], SOME 1)
            val msg =
               case !value of
                  NONE =>
                     let
                        val () = debug' ( concat [name, "(3.1.1)"]) (* Atomic 1 *)
                        val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(3.2.1)"], SOME 1)
                        val msg = NONE
                        val () = debug' ( concat [name, "(3.1.2)"]) (* Atomic 1 *)
                        val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(3.2.2)"], SOME 1)
                       val () = L.releaseCmlLock lock (S.tidNum ())
                        val () = S.atomicEnd ()
                        val () = debug' ( concat [name, "(3.1.3)"]) (* NonAtomic *)
                        val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(3.2.3)"])
                     in
                        msg
                     end
                | SOME x =>
                     let
                        val () = debug' ( concat [name, "(3.2.1)"]) (* Atomic 1 *)
                        val () = Assert.assertAtomic (fn () => concat ["SyncVar.", name, "(3.2.1)"], SOME 1)
                        val () = prio := 1
                        val () = doSwap value
                        (* XXX KC : Don't we need to propagate the msg here *)
                       val () = L.releaseCmlLock lock (S.tidNum ())
                        val () = S.atomicEnd ()
                        (* yield *)
                       (*val () = S.readyAndSwitchToNext (fn ()=>())*)
                        val () = debug' ( concat [name, "(3.2.2)"]) (* NonAtomic *)
                        val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(3.2.2)"])
                     in
                        SOME x
                     end
            val () = debug' ( concat [name, "(4)"]) (* NonAtomic *)
            val () = Assert.assertNonAtomic (fn () => concat ["SyncVar.", name, "(4)"])
         in
            msg
         end


      (** I-variables **)

      val iVar = newCell
      val sameIVar = sameCell

      fun iPut (cell, x) = gPut ("iPut", cell, x)
      local fun doGetSwap _ = ()
      in
         fun iGet cell = gSwap ("iGet", doGetSwap, cell)
         fun iGetEvt cell = gSwapEvt ("iGetEvt", doGetSwap, cell)
         fun iGetPoll cell = gSwapPoll ("iGetPoll", doGetSwap, cell)
      end

      (** M-variables **)

      val mVar = newCell
      fun mVarInit x = CELL {prio = ref 0, readQ = Q.new(), value = ref (SOME
        x), lock = L.initCmlLock ()}
      val sameMVar = sameCell

      fun mPut (cell, x) = gPut ("mPut", cell, x)
      local fun doTakeSwap value = value := NONE
      in
         fun mTake cell = gSwap ("mTake", doTakeSwap, cell)
         fun mTakeEvt cell = gSwapEvt ("mTakeEvt", doTakeSwap, cell)
         fun mTakePoll cell = gSwapPoll ("mTakePoll", doTakeSwap, cell)
      end
      local fun doGetSwap _ = ()
      in
         fun mGet cell = gSwap ("mGet", doGetSwap, cell)
         fun mGetEvt cell = gSwapEvt ("mGetEvt", doGetSwap, cell)
         fun mGetPoll cell = gSwapPoll ("mGetPoll", doGetSwap, cell)
      end
      local fun doSwapSwap x value = value := SOME x
      in
         fun mSwap (cell, x) = gSwap ("mSwap", doSwapSwap x, cell)
         fun mSwapEvt (cell, x) = gSwapEvt ("mSwap", doSwapSwap x, cell)
      end
   end
