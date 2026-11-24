% Ship at least 10 units using trucks and vans. An express delivery is
% optional but restricts the number of vans.
{ express }.
&sum { truck; van } >= 10.
&sum { truck } >= 0.
&sum { van } >= 0.
&sum { truck } <= 6.
&sum { van } <= 8.
&sum { truck } >= 6 :- express.
&sum { van } <= 4 :- express.
&minimize { 3*truck; 2*van }.
