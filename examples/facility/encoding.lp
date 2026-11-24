% Supply shops from factories minimizing the total cost.
%
% Which factories to open is a Boolean decision, the produced and shipped
% amounts are continuous, and the number of trucks on each route is an
% integer. An instance consists of facts
% - factory(F, O, C, P): opening factory F costs O, it can produce up to C
%   units, and producing a unit costs P,
% - demand(S, D): shop S needs D units,
% - route(F, S, T): trucks can drive from factory F to shop S at cost T, and
% - truck(K): a truck carries up to K units.

% Decide which factories to open.
{ open(F) } :- factory(F,_,_,_).

% Production is only possible at open factories.
&sum { make(F) } >= 0 :- factory(F,_,_,_).
&sum { make(F) } <= C :- open(F), factory(F,_,C,_).
&sum { make(F) } <= 0 :- factory(F,_,_,_), not open(F).

% The production of a factory is shipped to the shops.
&sum { ship(F,S) } >= 0 :- route(F,S,_).
&sum { ship(F,S) : route(F,S,_); -make(F) } = 0 :- factory(F,_,_,_).
&sum { ship(F,S) : route(F,S,_) } >= D :- demand(S,D).

% Shipping requires enough trucks.
&int { trucks(F,S) : route(F,S,_) }.
&sum { trucks(F,S) } >= 0 :- route(F,S,_).
&sum { K*trucks(F,S); -ship(F,S) } >= 0 :- route(F,S,_), truck(K).

% The costs for opening factories, producing units, and trucks.
&minimize { O : open(F), factory(F,O,_,_);
            P*make(F) : factory(F,_,_,P);
            T*trucks(F,S) : route(F,S,T) }.

#show open/1.
