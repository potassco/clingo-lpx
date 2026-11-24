% A small integer program whose relaxation has the fractional optimum y=14/5.
&int { x; y }.
&sum { -x; y } <= 1.
&sum { 3*x; 2*y } <= 12.
&sum { 2*x; 3*y } <= 12.
&sum { x } >= 0.
&sum { y } >= 0.
&maximize { y }.
