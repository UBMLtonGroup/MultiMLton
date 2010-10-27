signature THREAD =
sig
  type thread_id
  val getTid : unit -> thread_id
  val exit : unit -> unit
  val spawnHost : (unit -> unit) -> thread_id
  val spawnParasite : (unit -> unit) -> unit

  val spawn : (unit -> unit) -> thread_id
  val spawnOnProc : ((unit -> unit) * int) -> thread_id

end

signature THREAD_EXTRA =
sig
  include THREAD
  val createHost : (unit->unit) -> RepTypes.runnable_host
  val reifyHostFromParasite : RepTypes.parasite -> RepTypes.runnable_host
end