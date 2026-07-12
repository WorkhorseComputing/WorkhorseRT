(*
    MIT License

    Copyright (c) 2026 Humza Khan
    <mohammed.khan.2024@uni.strath.ac.uk>
    <https://github.com/humzak711>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*)

From Stdlib Require Import Lists.List.
From Stdlib Require Import Arith.Arith.
From RecordUpdate Require Import RecordUpdate.
From ExtLib.Data Require Import Option.
From Stdlib Require Import Bvector.
From Stdlib Require Import FSets.FMapList.
From Stdlib Require Import Structures.OrderedTypeEx.

Module NatMap := FMapList.Make(Nat_as_OT).

Record PriorityQ (V : Type) (nprio : nat) : Type := {
    qs : NatMap.t (list V);
    bmap : Bvector nprio;
}.

