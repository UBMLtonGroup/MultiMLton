(* Copyright (C) 1999-2005 Henry Cejtin, Matthew Fluet, Suresh
 *    Jagannathan, and Stephen Weeks.
 *
 * MLton is released under a BSD-style license.
 * See the file MLton-LICENSE for details.
 *)

structure Unit: UNIT =
struct

type t = unit

val equals = fn ((), ()) => true

fun layout() = Layout.str"()"

end