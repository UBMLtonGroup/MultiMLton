structure MLtonPacml : MLTON_PACML=
struct
  open Thread
  open Event
  open Channel
  open Lock
  open Main
  open Timeout
  structure Lock : LOCK = Lock
  structure SyncVar : SYNC_VAR = SyncVar
  structure Mailbox : MAILBOX = Mailbox
  structure Multicast : MULTICAST = Multicast 
  structure SimpleRPC : SIMPLE_RPC = SimpleRPC
end