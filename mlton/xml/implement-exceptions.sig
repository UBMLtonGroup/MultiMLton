(* Copyright (C) 1999-2006 Henry Cejtin, Matthew Fluet, Suresh
 *    Jagannathan, and Stephen Weeks.
 * Copyright (C) 1997-2000 NEC Research Institute.
 *
 * MLton is released under a BSD-style license.
 * See the file MLton-LICENSE for details.
 *)

signature IMPLEMENT_EXCEPTIONS_STRUCTS = 
   sig
      include SXML_TREE
   end

signature IMPLEMENT_EXCEPTIONS = 
   sig
      include IMPLEMENT_EXCEPTIONS_STRUCTS

      val doit: Program.t -> Program.t
   end
