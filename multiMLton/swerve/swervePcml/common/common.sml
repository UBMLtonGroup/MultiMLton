(*
Original Code - Copyright (c) 2001 Anthony L Shipman
MLton Port Modifications - Copyright (c) Ray Racine

Permission is granted to anyone to use this version of the software
for any purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

    1. Redistributions in source code must retain the above copyright
    notice, this list of conditions, and the following disclaimer.

    2. The origin of this software must not be misrepresented; you must
    not claim that you wrote the original software. If you use this
    software in a product, an acknowledgment in the product documentation
    would be appreciated but is not required.

    3. If any files are modified, you must cause the modified files to
    carry prominent notices stating that you changed the files and the
    date of any change.

Disclaimer

    THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
    WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

Modification History
====================
Ray Racine 6/3/2005 - MLton Port and idiomatic fixups.
*)


(*  Useful common declarations. *)

structure Common =
struct

    val connBackLog = ref 2000
    exception FatalX
    exception InternalError of string

    fun printToErr s = MLton.Thread.atomically ( fn () =>
						    ( TextIO.output ( TextIO.stdErr, s );
    						      TextIO.flushOut ( TextIO.stdErr ) ) )

    (* These shutdown the server with the given status code. *)
    fun success () = MLton.RunPCML.shutdown OS.Process.success
    fun fail () = ( printToErr "Aborting\n";
		    MLton.RunPCML.shutdown OS.Process.failure )

    datatype SrcPos = SrcPos of
    	     { file: string,
	       line: int,			(* first line is line 1 *)
	       col:  int }			(* left-most column is col 1 *)


    fun formatPos ( SrcPos { file, line, col } ) =
	concat [ file, "@", Int.toString line, ".", Int.toString col ]

    (* Construct a general purpose map with strings as keys. *)

    structure STRT_key =
    struct
      type hash_key = string
      val hashVal = HashString.hashString
      fun sameKey ( s1, s2 ) = ( s1 = s2 )
    end

    exception NotFound

    structure STRT = HashTableFn ( STRT_key )

    fun isVal   c1 c2 = (c1 = c2)
    fun isntVal c1 c2 = (c1 <> c2)

    fun upperCase str = String.map Char.toUpper str

end