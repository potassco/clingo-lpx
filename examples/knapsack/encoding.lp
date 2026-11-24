% Pack copies of items into a knapsack with limited resources maximizing the
% total value.
%
% An instance consists of facts
% - item(I, V, S): up to S copies of item I with value V each are available,
% - requires(I, R, A): a copy of item I needs A units of resource R,
% - capacity(R, C): there are C units of resource R, and
% - conflict(I, J): items I and J cannot be packed together.

% Decide which items to pack (at least one copy each).
{ pack(I) } :- item(I,_,_).
:- conflict(I,J), pack(I), pack(J).

% The number of copies is an integer.
&int { copies(I) : item(I,_,_) }.
&sum { copies(I) } >= 1 :- pack(I).
&sum { copies(I) } <= S :- pack(I), item(I,_,S).
&sum { copies(I) } = 0 :- item(I,_,_), not pack(I).

% The copies must fit into the resources.
&sum { A*copies(I) : requires(I,R,A) } <= C :- capacity(R,C).

&maximize { V*copies(I) : item(I,V,_) }.

#show pack/1.
