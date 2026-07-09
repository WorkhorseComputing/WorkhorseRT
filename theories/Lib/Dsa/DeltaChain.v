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
Require Import Lia.

Import ListNotations.

(* ==================================================================================================================== *)
(* Definitions *)
(* ==================================================================================================================== *)

Record DeltaNode (V : Type) : Type := {
    expectedEpoch : nat;
    delta : nat;
    payload : V; 
}.

Arguments expectedEpoch {V}.
Arguments delta {V}. 
Arguments payload {V}.

Definition DeltaChainList (V : Type) := list (DeltaNode V).

Record DeltaChain (V : Type) : Type := {
    chain : DeltaChainList V; 
    currentEpoch : nat;
}.

Arguments chain {V}.
Arguments currentEpoch {V}.

Definition makeDeltaNode {V : Type} (dlt : nat) (p : V) := {| expectedEpoch := 0; delta := dlt; payload := p |}.
Definition makeDeltaChain {V : Type} (c : DeltaChainList V) := {| chain := c; currentEpoch := 0 |}. 

Fixpoint ForallDeltaChainList {V : Type} (c : DeltaChainList V) (P : DeltaNode V -> Prop) : Prop :=
    match c with 
    | [] => True
    | h :: t => P h /\ ForallDeltaChainList t P
    end.
    
Fixpoint deltaChainListOrdered {V : Type} (c : DeltaChainList V) (e : nat) : Prop :=
    match c with 
    | [] => True 
    | h :: t => e <= h.(expectedEpoch) /\ deltaChainListOrdered t h.(expectedEpoch)
    end.

Fixpoint deltaChainListDeltasValid {V : Type} (c : DeltaChainList V) (off : nat) : Prop :=
    match c with
    | [] => True
    | h :: t => let eat := off + h.(delta) in eat = h.(expectedEpoch) /\ deltaChainListDeltasValid t eat
    end.

Definition deltaChainCurrentEpochValid {V : Type} (dc : DeltaChain V) : Prop :=
    match dc.(chain) with
    | [] => True
    | h :: _ =>  dc.(currentEpoch) + h.(delta) = h.(expectedEpoch)
    end.

Definition deltaChainOrdered {V : Type} (dc : DeltaChain V) : Prop :=
    match dc.(chain) with
    | [] => True
    | h :: t => deltaChainListOrdered t h.(expectedEpoch)
    end.

Definition deltaChainDeltasValid {V : Type} (dc : DeltaChain V) : Prop := 
    deltaChainListDeltasValid dc.(chain) dc.(currentEpoch).
    
Definition ForallDeltaChain {V : Type} (dc : DeltaChain V) (P : DeltaNode V -> Prop) : Prop := 
    ForallDeltaChainList dc.(chain) P.

Inductive DeltaChainInv {V : Type} : DeltaChain V -> Prop :=
    | DELTACHAIN_E : forall dc,     dc.(chain) = [] -> DeltaChainInv dc
    | DELTACHAIN_C : forall dc,         dc.(chain) <> [] -> 
                                        deltaChainCurrentEpochValid dc ->
                                        deltaChainOrdered dc ->
                                        deltaChainDeltasValid dc -> 
                                        DeltaChainInv dc.

(* ==================================================================================================================== *)
(* Operations *)
(* ==================================================================================================================== *)

Fixpoint deltaChainListInsert {V : Type} (c :  DeltaChainList V) (d : DeltaNode V) : DeltaChainList V := 
    match c with 
    | [] =>     [d]
    | h :: t => let dd := d.(delta) in
                let hd := h.(delta) in
                    if dd <=? hd then d :: h <| delta := hd - dd |> :: t 
                    else h :: deltaChainListInsert t (d <| delta := dd - hd |>)

    end.

Fixpoint deltaChainListTick {V : Type} (c :  DeltaChainList V) : DeltaChainList V := 
    match c with 
    | [] =>     []
    | h :: t => if h.(delta) =? 0 then deltaChainListTick t else h <| delta := h.(delta) - 1 |> :: t
    end.

Definition deltaChainInsert {V : Type} (dc : DeltaChain V) (d : DeltaNode V) : DeltaChain V := dc <| 
    chain := deltaChainListInsert dc.(chain) (d <| expectedEpoch := dc.(currentEpoch) + d.(delta) |>)
|>.

Definition deltaChainTick {V : Type} (dc : DeltaChain V) : DeltaChain V := {|
    chain := deltaChainListTick dc.(chain);
    currentEpoch := dc.(currentEpoch) + 1;
|}.

Definition deltaChainPopExpired {V : Type} (dc : DeltaChain V) : (DeltaChain V * option (DeltaNode V))  :=
    match dc.(chain) with
    | [] => (dc, None)
    | h :: t => if h.(delta) =? 0 then (dc <| chain := t |>, Some h) else (dc, None)   
    end.

Definition deltaChainPop {V : Type} (dc : DeltaChain V) : (DeltaChain V * option (DeltaNode V))  :=
    match dc.(chain) with
    | [] => (dc, None)
    | h :: [] => (dc <| chain := [] |>, Some h)
    | h :: h' :: t => ( dc <| chain := (h' <| delta := h.(delta) + h'.(delta) |>) :: t |>, Some h)  
    end.

Definition deltaChainIsEmpty {V : Type} (dc : DeltaChain V) : bool := 
    match dc.(chain) with
    | [] => true
    | _ => false
    end.

Definition deltaChainPeek {V : Type} (dc : DeltaChain V) : option (DeltaNode V) :=
    match dc.(chain) with 
    | [] => None 
    | h :: t => Some h
    end.

Definition deltaChainTimeUntil {V : Type} (dc : DeltaChain V) (d : DeltaNode V) : nat := 
    d.(expectedEpoch) - dc.(currentEpoch).

(* ==================================================================================================================== *)
(* Proofs *)
(* ==================================================================================================================== *)

Lemma delta_chain_list_insert_inc_length : forall (V : Type) (c : DeltaChainList V) (d : DeltaNode V),
    length (deltaChainListInsert c d) = S (length c).
Proof.
    intros v c.
    induction c; intros d; simpl.
    - reflexivity.
    - destruct (delta d <=? delta a) eqn:Hcmp; simpl.
    -- reflexivity.
    -- rewrite IHc. reflexivity.
Qed.

Lemma delta_chain_list_insert_preserves_order : forall (V : Type) (c : DeltaChainList V) (d : DeltaNode V) (ce : nat),
    ce + d.(delta) = d.(expectedEpoch) -> 
    deltaChainListOrdered c ce -> 
    deltaChainListDeltasValid c ce -> 
    deltaChainListOrdered (deltaChainListInsert c d) ce.
Proof.
    intros v c.
    induction c; intros; simpl.
    - auto.
    -- lia.
    - destruct (delta d <=? delta a) eqn: Hcmp; simpl in *.
    -- repeat split.
    --- lia.
    --- apply Nat.leb_le in Hcmp. lia.
    --- apply H0.
    -- destruct H0 as [Hbound Hord]. destruct H1 as [Heq_a Hdeltas_c]. simpl in *. apply Nat.leb_gt in Hcmp. split.
    --- auto.
    --- apply IHc; simpl.
    ---- lia.
    ---- auto.
    ---- rewrite <- Heq_a. auto.
Qed.      

Lemma delta_chain_list_insert_preserves_deltas : forall (V : Type) (c : DeltaChainList V) (d : DeltaNode V) (ce : nat),
    ce + d.(delta) = d.(expectedEpoch) -> 
    deltaChainListOrdered c ce -> 
    deltaChainListDeltasValid c ce -> 
    deltaChainListDeltasValid (deltaChainListInsert c d) ce.
Proof.
    intros v c.
    induction c; intros; simpl.
    - auto.
    - destruct (delta d <=? delta a) eqn: Hcmp; repeat split; simpl in *.
    -- auto.
    -- rewrite Nat.leb_le in Hcmp. lia.
    -- destruct H1 as [Heq_a Hdeltas]; simpl.
    --- apply Nat.leb_le in Hcmp. replace (ce + delta d + (delta a - delta d)) with (ce + delta a) by lia. auto.
    -- apply H1.
    -- apply IHc; simpl. 
    --- apply Nat.leb_gt in Hcmp. replace (ce + delta a + (delta d - delta a)) with (ce + (delta d)) by lia. auto.
    --- destruct H1. rewrite H1. apply H0.
    --- apply H1. 
Qed.      


Theorem delta_chain_insert_inc_length : forall (V : Type) (dc : DeltaChain V) (d : DeltaNode V),
    length (deltaChainInsert dc d).(chain) = length dc.(chain) + 1.
Proof.
    intros.
    unfold deltaChainInsert.
    induction (chain dc); simpl.
    - reflexivity.
    - destruct (delta d <=? delta a) eqn:Hcmp; simpl.
    -- lia.
    -- rewrite delta_chain_list_insert_inc_length. lia.
Qed.

Theorem delta_chain_insert_preserves_inv : forall (V : Type) (dc : DeltaChain V) (d : DeltaNode V),
    DeltaChainInv dc ->  DeltaChainInv (deltaChainInsert dc d).
Proof.
    intros v dc d Hinv. destruct Hinv; unfold deltaChainInsert; simpl in *.
    - rewrite H. simpl. apply DELTACHAIN_C; simpl.
    -- discriminate.
    -- unfold deltaChainCurrentEpochValid. simpl. lia.
    -- unfold deltaChainOrdered. simpl. reflexivity.
    -- unfold deltaChainDeltasValid. simpl. lia.
    
    - apply DELTACHAIN_C; simpl.
    -- unfold deltaChainListInsert. destruct (chain dc) eqn : Hchain; simpl.
    --- contradict H. reflexivity.
    --- destruct (delta d <=? delta d0); simpl; discriminate.

    -- unfold deltaChainCurrentEpochValid. destruct (chain dc) eqn : Hchain; simpl.
    --- lia.
    --- destruct (delta d <=? delta d0); simpl. 
    ---- lia. 
    ---- unfold deltaChainCurrentEpochValid in H0. rewrite Hchain in H0. apply H0.
    
    -- unfold deltaChainOrdered. simpl. destruct (chain dc) eqn: Hchain; simpl.
    --- reflexivity.
    --- destruct (delta d <=? delta d0) eqn: Hcmp; simpl.
    ---- unfold deltaChainDeltasValid in H2. rewrite Hchain in H2. simpl in *. destruct H2. split.
    ----- apply Nat.leb_le in Hcmp. lia.
    ----- unfold deltaChainOrdered in H1. rewrite Hchain in H1. apply H1.
    ----  unfold deltaChainDeltasValid in H2.
          unfold deltaChainListDeltasValid in H2. rewrite Hchain in H2. simpl in H2. 
          destruct H2 as [Heq_0 Hvalid_d1]; simpl.
          apply Nat.leb_gt in Hcmp.
          apply delta_chain_list_insert_preserves_order; simpl.
    ----- lia.
    ----- unfold deltaChainOrdered in H1. rewrite Hchain in H1. apply H1.
    ----- rewrite <- Heq_0. apply Hvalid_d1.

    -- unfold deltaChainDeltasValid in *. simpl. destruct (chain dc) eqn : Hchain; simpl in *. 
    --- auto.
    --- destruct (delta d <=? delta d0) eqn: Hcmp; simpl; split.
    ---- reflexivity.
    ---- apply Nat.leb_le in Hcmp. 
          replace (currentEpoch dc + delta d + (delta d0 - delta d)) with (currentEpoch dc + delta d0) by lia.
          auto.
    ---- apply H2.
    ---- apply delta_chain_list_insert_preserves_deltas; simpl.
    ----- apply Nat.leb_gt in Hcmp. 
          replace (currentEpoch dc + delta d0 + (delta d - delta d0)) with (currentEpoch dc + delta d) by lia.
          reflexivity.
    ----- unfold deltaChainOrdered in H1. rewrite Hchain in H1. destruct H2. rewrite H2. apply H1.
    ----- apply H2.
Qed. 

Theorem delta_chain_tick_preserves_inv : forall (V : Type) (dc : DeltaChain V),
    DeltaChainInv dc -> DeltaChainInv (deltaChainTick dc).
Proof.
    intros v dc Hinv.
    destruct Hinv eqn: Hinv' ; unfold deltaChainTick; simpl in *.
    - apply DELTACHAIN_E. simpl. rewrite e. reflexivity.
    - destruct dc as [c ce]; simpl in *.
    -- destruct c eqn: Hc. 
    --- exfalso. apply n. reflexivity.
    --- subst. unfold deltaChainListTick. 

Theorem delta_chain_pop_expired_empty_valid : forall (V : Type) (dc : DeltaChain V), 
    snd (deltaChainPopExpired dc) = None <-> 
    (forall d t, dc.(chain) <> d :: t \/ d.(delta) <> 0).
Proof.
    intros.
    repeat split; intros; unfold deltaChainPopExpired in H; simpl in *.
    - destruct (chain dc) eqn: Hchain; simpl. 
    -- left. discriminate.
    -- destruct (Nat.eqb (delta d0) (delta d)) eqn:Heq; simpl.
    --- right. apply Nat.eqb_eq in Heq. rewrite <- Heq. destruct (delta d0 =? 0) eqn:Hd0; simpl.
    ---- apply Nat.eqb_eq in Hd0. rewrite Hd0. discriminate.
    ---- apply Nat.eqb_neq in Hd0. apply Hd0.
    --- apply Nat.eqb_neq in Heq. left. intros Hlisteq. inversion Hlisteq. subst. apply Heq. reflexivity.
    - unfold deltaChainPopExpired. destruct (chain dc) eqn:Hchain; simpl.
    -- reflexivity.
    -- specialize (H d d0). destruct H; simpl.
    --- contradict H. reflexivity.
    --- destruct (delta d =? 0) eqn: Hd0; simpl. 
    ---- apply Nat.eqb_eq in Hd0. contradict H. apply Hd0.
    ---- reflexivity.
Qed.

Theorem delta_chain_pop_expired_some_valid : forall (V : Type) (dc : DeltaChain V) (d : DeltaNode V),
    (exists t, dc.(chain) = d :: t /\ d.(delta) = 0) <-> 

    (snd (deltaChainPopExpired dc) = Some d /\
     length (fst (deltaChainPopExpired dc)).(chain) = (length dc.(chain) - 1)).
Proof.
    intros.
    repeat split; intros; unfold deltaChainPopExpired; simpl in *; destruct (chain dc) eqn: Hchain; simpl.

    - destruct H as [Hlist [Hnil]]. discriminate Hnil.
    - destruct (delta d0) eqn: Hd0; simpl; destruct (chain dc) in Hchain; try discriminate. 
    -- destruct H as [Hlist [Heq]]. inversion Heq. reflexivity.
    -- destruct H as [Hlist [Heq]]. inversion Heq. subst. contradict H. lia.

    - rewrite Hchain. reflexivity.
    - destruct H as [Hlist [Heq]]. destruct (delta d0) eqn: Hd0; simpl. 
    -- lia.
    -- inversion Heq. subst. rewrite H in Hd0. discriminate Hd0.
    - destruct H as [Hexp]. unfold deltaChainPopExpired in *. 
      destruct (chain dc) eqn :Hchain2 in Hexp; destruct (chain dc) eqn : Hchain3 in *; simpl in *; inversion Hexp; 
      destruct (delta d0 =? 0) eqn: Hd0 in *; simpl in *.
    -- inversion Hexp. subst. apply Nat.eqb_eq in Hd0. exists []. split.
    --- discriminate Hchain2.
    --- apply Hd0.
    -- discriminate H1.
    -- inversion Hchain.
    -- discriminate H1.
    - rewrite <- Hchain. unfold deltaChainPopExpired in H. destruct (chain dc) eqn: Hchain2 in H; simpl in *.
    -- inversion H. inversion H0.
    -- destruct (delta d2 =? 0) eqn: Hd20 in H; rewrite Hchain2 in Hchain; inversion Hchain.
    --- apply Nat.eqb_eq in Hd20. subst. destruct H. inversion H. subst. exists d1. auto. 
    --- destruct H. inversion H.
Qed.

Conjecture delta_chain_pop_expired_preserves_inv : forall (V : Type) (dc : DeltaChain V),
    DeltaChainInv dc -> DeltaChainInv (fst (deltaChainPopExpired dc)).

Theorem delta_chain_pop_valid : forall (V : Type) (dc : DeltaChain V),
    (dc.(chain) = [] <-> snd (deltaChainPop dc) = None) /\
    (dc.(chain) <> [] <-> (exists d t, snd (deltaChainPop dc) = Some d /\ dc.(chain) = d :: t)).
Proof.
    intros.
    repeat split; intros; simpl in *.
    - unfold deltaChainPop. destruct (chain dc) eqn: Hchain; simpl. 
    -- reflexivity.
    -- discriminate H.
    - unfold deltaChainPop in H. destruct (chain dc) eqn: Hchain in H; simpl.
    -- auto.
    -- destruct d0 in H; simpl in *; discriminate H.
    - unfold deltaChainPop. destruct (chain dc) eqn: Hchain; simpl.
    -- contradict H. reflexivity.
    -- exists d. exists d0. split; simpl in *.
    --- destruct d0 eqn: Hd0; simpl; reflexivity.
    --- reflexivity.
    - unfold deltaChainPop in H. destruct (chain dc) eqn: Hchain in H; simpl. 
    -- intros HchainF. destruct H as [Hlist [Hsome [Hcontra]]]. discriminate H.
    -- destruct d0 eqn: Hd0 in H; simpl in *.
    --- destruct H as [Hn [Hsome [Hl]]]. subst. rewrite H in Hchain. rewrite Hchain. discriminate.
    --- rewrite Hd0 in Hchain. rewrite <- Hchain in H. destruct H as [Hn [Hsome [Hl]]].
    ---- rewrite Hchain. discriminate. 
Qed.

Conjecture delta_chain_pop_preserves_inv : forall (V : Type) (dc : DeltaChain V),
    DeltaChainInv dc -> DeltaChainInv (fst (deltaChainPop dc)).

Theorem delta_chain_is_empty_valid : forall (V : Type) (dc : DeltaChain V), 
    (dc.(chain) = [] <-> deltaChainIsEmpty dc = true) /\ 
    (dc.(chain) <> [] <-> deltaChainIsEmpty dc = false).
Proof.
    intros.
    repeat split; intros; simpl in *.
    - unfold deltaChainIsEmpty. rewrite H. reflexivity.
    - unfold deltaChainIsEmpty in H. destruct (chain dc). reflexivity. discriminate.
    - unfold deltaChainIsEmpty. destruct (chain dc). contradict H. reflexivity. reflexivity.
    - unfold deltaChainIsEmpty in H. destruct (chain dc); discriminate.
Qed.

Theorem delta_chain_peek_valid : forall (V : Type) (dc : DeltaChain V),
    (dc.(chain) = [] <-> deltaChainPeek dc = None) /\
    (dc.(chain) <> [] <-> (exists d t, deltaChainPeek dc = Some d /\ dc.(chain) = d :: t)).
Proof.
    intros.
    repeat split; intros; simpl in *.
    - unfold deltaChainPeek. rewrite H. reflexivity.
    - unfold deltaChainPeek in H. destruct (chain dc) eqn: Hchain; simpl.
    -- reflexivity.
    -- discriminate.
    - unfold deltaChainPeek.  destruct (chain dc) eqn: Hchain; simpl.
    -- contradict H. reflexivity.
    -- exists d. exists d0. auto.
    - destruct H as [x [chain_x [dc_peek dc_chain ] ] ].
    -- rewrite dc_chain. discriminate.
Qed.   