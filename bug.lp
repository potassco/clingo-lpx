{ releasedAt(value,0) }.
{ releasedAt(value,1) }.
{ releasedAt(value,2) }.
{ releasedAt(value,3) }.
 :- releasedAt(value,1); not releasedAt(value,0).
 :- releasedAt(value,2); not releasedAt(value,1).
 :- releasedAt(value,3); not releasedAt(value,2).
#sum+ { 1,0,sum(((1,(time,0)),),lt,10): sum(((1,(time,0)),),lt,10); 1,0,sum(((1,(time,0)),),eq,10): sum(((1,(time,0)),),eq,10); 1,0,sum(((1,(time,0)),),gt,10): sum(((1,(time,0)),),gt,10) } = 1.
#sum+ { 1,0,sum(((1,(time,1)),),lt,10): sum(((1,(time,1)),),lt,10); 1,0,sum(((1,(time,1)),),eq,10): sum(((1,(time,1)),),eq,10); 1,0,sum(((1,(time,1)),),gt,10): sum(((1,(time,1)),),gt,10) } = 1.
#sum+ { 1,0,sum(((1,(time,2)),),lt,10): sum(((1,(time,2)),),lt,10); 1,0,sum(((1,(time,2)),),eq,10): sum(((1,(time,2)),),eq,10); 1,0,sum(((1,(time,2)),),gt,10): sum(((1,(time,2)),),gt,10) } = 1.
#sum+ { 1,0,sum(((1,(time,3)),),lt,10): sum(((1,(time,3)),),lt,10); 1,0,sum(((1,(time,3)),),eq,10): sum(((1,(time,3)),),eq,10); 1,0,sum(((1,(time,3)),),gt,10): sum(((1,(time,3)),),gt,10) } = 1.
#sum+ { 1,0,sum(((1,(time,0)),),lt,20): sum(((1,(time,0)),),lt,20); 1,0,sum(((1,(time,0)),),eq,20): sum(((1,(time,0)),),eq,20); 1,0,sum(((1,(time,0)),),gt,20): sum(((1,(time,0)),),gt,20) } = 1.
#sum+ { 1,0,sum(((1,(time,1)),),lt,20): sum(((1,(time,1)),),lt,20); 1,0,sum(((1,(time,1)),),eq,20): sum(((1,(time,1)),),eq,20); 1,0,sum(((1,(time,1)),),gt,20): sum(((1,(time,1)),),gt,20) } = 1.
#sum+ { 1,0,sum(((1,(time,2)),),lt,20): sum(((1,(time,2)),),lt,20); 1,0,sum(((1,(time,2)),),eq,20): sum(((1,(time,2)),),eq,20); 1,0,sum(((1,(time,2)),),gt,20): sum(((1,(time,2)),),gt,20) } = 1.
#sum+ { 1,0,sum(((1,(time,3)),),lt,20): sum(((1,(time,3)),),lt,20); 1,0,sum(((1,(time,3)),),eq,20): sum(((1,(time,3)),),eq,20); 1,0,sum(((1,(time,3)),),gt,20): sum(((1,(time,3)),),gt,20) } = 1.
#sum+ { 1,0,sum(((1,(time,0)),),lt,30): sum(((1,(time,0)),),lt,30); 1,0,sum(((1,(time,0)),),eq,30): sum(((1,(time,0)),),eq,30); 1,0,sum(((1,(time,0)),),gt,30): sum(((1,(time,0)),),gt,30) } = 1.
#sum+ { 1,0,sum(((1,(time,1)),),lt,30): sum(((1,(time,1)),),lt,30); 1,0,sum(((1,(time,1)),),eq,30): sum(((1,(time,1)),),eq,30); 1,0,sum(((1,(time,1)),),gt,30): sum(((1,(time,1)),),gt,30) } = 1.
#sum+ { 1,0,sum(((1,(time,2)),),lt,30): sum(((1,(time,2)),),lt,30); 1,0,sum(((1,(time,2)),),eq,30): sum(((1,(time,2)),),eq,30); 1,0,sum(((1,(time,2)),),gt,30): sum(((1,(time,2)),),gt,30) } = 1.
#sum+ { 1,0,sum(((1,(time,3)),),lt,30): sum(((1,(time,3)),),lt,30); 1,0,sum(((1,(time,3)),),eq,30): sum(((1,(time,3)),),eq,30); 1,0,sum(((1,(time,3)),),gt,30): sum(((1,(time,3)),),gt,30) } = 1.
#sum+ { 1,0,sum(((1,(time,4)),),lt,30): sum(((1,(time,4)),),lt,30); 1,0,sum(((1,(time,4)),),eq,30): sum(((1,(time,4)),),eq,30); 1,0,sum(((1,(time,4)),),gt,30): sum(((1,(time,4)),),gt,30) } = 1.
#sum+ { 1,0,sum(((1,(time,0)),),lt,40): sum(((1,(time,0)),),lt,40); 1,0,sum(((1,(time,0)),),eq,40): sum(((1,(time,0)),),eq,40); 1,0,sum(((1,(time,0)),),gt,40): sum(((1,(time,0)),),gt,40) } = 1.
#sum+ { 1,0,sum(((1,(time,1)),),lt,40): sum(((1,(time,1)),),lt,40); 1,0,sum(((1,(time,1)),),eq,40): sum(((1,(time,1)),),eq,40); 1,0,sum(((1,(time,1)),),gt,40): sum(((1,(time,1)),),gt,40) } = 1.
#sum+ { 1,0,sum(((1,(time,2)),),lt,40): sum(((1,(time,2)),),lt,40); 1,0,sum(((1,(time,2)),),eq,40): sum(((1,(time,2)),),eq,40); 1,0,sum(((1,(time,2)),),gt,40): sum(((1,(time,2)),),gt,40) } = 1.
#sum+ { 1,0,sum(((1,(time,3)),),lt,40): sum(((1,(time,3)),),lt,40); 1,0,sum(((1,(time,3)),),eq,40): sum(((1,(time,3)),),eq,40); 1,0,sum(((1,(time,3)),),gt,40): sum(((1,(time,3)),),gt,40) } = 1.
#sum+ { 1,0,sum(((1,(time,4)),),lt,40): sum(((1,(time,4)),),lt,40); 1,0,sum(((1,(time,4)),),eq,40): sum(((1,(time,4)),),eq,40); 1,0,sum(((1,(time,4)),),gt,40): sum(((1,(time,4)),),gt,40) } = 1.
map(10,0) :- sum(((1,(time,0)),),eq,10).
map(10,1) :- sum(((1,(time,1)),),eq,10).
map(10,3) :- sum(((1,(time,3)),),eq,10).
map(20,0) :- sum(((1,(time,0)),),eq,20).
map(20,1) :- sum(((1,(time,1)),),eq,20).
map(20,2) :- sum(((1,(time,2)),),eq,20).
map(30,0) :- sum(((1,(time,0)),),eq,30).
map(30,1) :- sum(((1,(time,1)),),eq,30).
map(30,2) :- sum(((1,(time,2)),),eq,30).
map(30,3) :- sum(((1,(time,3)),),eq,30).
map(30,4) :- sum(((1,(time,4)),),eq,30).
map(40,0) :- sum(((1,(time,0)),),eq,40).
map(40,1) :- sum(((1,(time,1)),),eq,40).
map(40,2) :- sum(((1,(time,2)),),eq,40).
map(40,3) :- sum(((1,(time,3)),),eq,40).
happens(increment,0) :- map(10,0).
happens(increment,1) :- map(10,1).
happens(increment,3) :- map(10,3).
happens(increment,0) :- map(20,0).
happens(increment,1) :- map(20,1).
happens(increment,2) :- map(20,2).
happens(reset,1) :- map(30,1).
happens(reset,2) :- map(30,2).
happens(reset,3) :- map(30,3).
happens(increment,0) :- map(40,0).
happens(increment,1) :- map(40,1).
happens(increment,2) :- map(40,2).
happens(increment,3) :- map(40,3).
initiated1(value,0) :- happens(increment,0).
initiated1(value,1) :- happens(increment,1).
initiated1(value,2) :- happens(increment,2).
&sum { (value,0) } = (value,(0+1)) :- not releasedAt(value,1); not initiated1(value,0).
&sum { (value,1) } = (value,(1+1)) :- not releasedAt(value,2); not initiated1(value,1).
&sum { (value,2) } = (value,(2+1)) :- not releasedAt(value,3); not initiated1(value,2).
&sum { (1*(time,1)) } < 10 :- sum(((1,(time,1)),),lt,10).
&sum { (1*(time,2)) } < 10 :- sum(((1,(time,2)),),lt,10).
&sum { (1*(time,3)) } < 10 :- sum(((1,(time,3)),),lt,10).
&sum { (1*(time,2)) } < 20 :- sum(((1,(time,2)),),lt,20).
&sum { (1*(time,3)) } < 20 :- sum(((1,(time,3)),),lt,20).
&sum { (1*(time,0)) } < 30 :- sum(((1,(time,0)),),lt,30).
&sum { (1*(time,3)) } < 30 :- sum(((1,(time,3)),),lt,30).
&sum { (1*(time,4)) } < 30 :- sum(((1,(time,4)),),lt,30).
&sum { (1*(time,4)) } < 40 :- sum(((1,(time,4)),),lt,40).
&sum { (1*(time,0)) } = 10 :- sum(((1,(time,0)),),eq,10).
&sum { (1*(time,1)) } = 10 :- sum(((1,(time,1)),),eq,10).
&sum { (1*(time,2)) } = 10 :- sum(((1,(time,2)),),eq,10).
&sum { (1*(time,3)) } = 10 :- sum(((1,(time,3)),),eq,10).
&sum { (1*(time,0)) } = 20 :- sum(((1,(time,0)),),eq,20).
&sum { (1*(time,1)) } = 20 :- sum(((1,(time,1)),),eq,20).
&sum { (1*(time,2)) } = 20 :- sum(((1,(time,2)),),eq,20).
&sum { (1*(time,3)) } = 20 :- sum(((1,(time,3)),),eq,20).
&sum { (1*(time,0)) } = 30 :- sum(((1,(time,0)),),eq,30).
&sum { (1*(time,1)) } = 30 :- sum(((1,(time,1)),),eq,30).
&sum { (1*(time,2)) } = 30 :- sum(((1,(time,2)),),eq,30).
&sum { (1*(time,3)) } = 30 :- sum(((1,(time,3)),),eq,30).
&sum { (1*(time,4)) } = 30 :- sum(((1,(time,4)),),eq,30).
&sum { (1*(time,0)) } = 40 :- sum(((1,(time,0)),),eq,40).
&sum { (1*(time,1)) } = 40 :- sum(((1,(time,1)),),eq,40).
&sum { (1*(time,2)) } = 40 :- sum(((1,(time,2)),),eq,40).
&sum { (1*(time,3)) } = 40 :- sum(((1,(time,3)),),eq,40).
&sum { (1*(time,4)) } = 40 :- sum(((1,(time,4)),),eq,40).
&sum { (1*(time,0)) } > 20 :- sum(((1,(time,0)),),gt,20).
&sum { (1*(time,0)) } > 30 :- sum(((1,(time,0)),),gt,30).
&sum { (1*(time,1)) } > 30 :- sum(((1,(time,1)),),gt,30).
&sum { (1*(time,0)) } > 40 :- sum(((1,(time,0)),),gt,40).
&sum { (1*(time,1)) } > 40 :- sum(((1,(time,1)),),gt,40).
&sum { (1*(time,2)) } > 40 :- sum(((1,(time,2)),),gt,40).
sum(((1,(time,0)),),leq,10) :- sum(((1,(time,0)),),lt,10).
sum(((1,(time,1)),),leq,10) :- sum(((1,(time,1)),),lt,10).
sum(((1,(time,2)),),leq,10) :- sum(((1,(time,2)),),lt,10).
sum(((1,(time,3)),),leq,10) :- sum(((1,(time,3)),),lt,10).
sum(((1,(time,0)),),leq,20) :- sum(((1,(time,0)),),lt,20).
sum(((1,(time,1)),),leq,20) :- sum(((1,(time,1)),),lt,20).
sum(((1,(time,2)),),leq,20) :- sum(((1,(time,2)),),lt,20).
sum(((1,(time,3)),),leq,20) :- sum(((1,(time,3)),),lt,20).
sum(((1,(time,0)),),leq,30) :- sum(((1,(time,0)),),lt,30).
sum(((1,(time,1)),),leq,30) :- sum(((1,(time,1)),),lt,30).
sum(((1,(time,2)),),leq,30) :- sum(((1,(time,2)),),lt,30).
sum(((1,(time,3)),),leq,30) :- sum(((1,(time,3)),),lt,30).
sum(((1,(time,4)),),leq,30) :- sum(((1,(time,4)),),lt,30).
sum(((1,(time,0)),),leq,40) :- sum(((1,(time,0)),),lt,40).
sum(((1,(time,1)),),leq,40) :- sum(((1,(time,1)),),lt,40).
sum(((1,(time,2)),),leq,40) :- sum(((1,(time,2)),),lt,40).
sum(((1,(time,3)),),leq,40) :- sum(((1,(time,3)),),lt,40).
sum(((1,(time,4)),),leq,40) :- sum(((1,(time,4)),),lt,40).
sum(((1,(time,0)),),leq,10) :- sum(((1,(time,0)),),eq,10).
sum(((1,(time,1)),),leq,10) :- sum(((1,(time,1)),),eq,10).
sum(((1,(time,2)),),leq,10) :- sum(((1,(time,2)),),eq,10).
sum(((1,(time,3)),),leq,10) :- sum(((1,(time,3)),),eq,10).
sum(((1,(time,0)),),leq,20) :- sum(((1,(time,0)),),eq,20).
sum(((1,(time,1)),),leq,20) :- sum(((1,(time,1)),),eq,20).
sum(((1,(time,2)),),leq,20) :- sum(((1,(time,2)),),eq,20).
sum(((1,(time,3)),),leq,20) :- sum(((1,(time,3)),),eq,20).
sum(((1,(time,0)),),leq,30) :- sum(((1,(time,0)),),eq,30).
sum(((1,(time,1)),),leq,30) :- sum(((1,(time,1)),),eq,30).
sum(((1,(time,2)),),leq,30) :- sum(((1,(time,2)),),eq,30).
sum(((1,(time,3)),),leq,30) :- sum(((1,(time,3)),),eq,30).
sum(((1,(time,4)),),leq,30) :- sum(((1,(time,4)),),eq,30).
sum(((1,(time,0)),),leq,40) :- sum(((1,(time,0)),),eq,40).
sum(((1,(time,1)),),leq,40) :- sum(((1,(time,1)),),eq,40).
sum(((1,(time,2)),),leq,40) :- sum(((1,(time,2)),),eq,40).
sum(((1,(time,3)),),leq,40) :- sum(((1,(time,3)),),eq,40).
sum(((1,(time,4)),),leq,40) :- sum(((1,(time,4)),),eq,40).
sum(((1,(time,0)),),geq,10) :- sum(((1,(time,0)),),eq,10).
sum(((1,(time,1)),),geq,10) :- sum(((1,(time,1)),),eq,10).
sum(((1,(time,2)),),geq,10) :- sum(((1,(time,2)),),eq,10).
sum(((1,(time,3)),),geq,10) :- sum(((1,(time,3)),),eq,10).
sum(((1,(time,0)),),geq,20) :- sum(((1,(time,0)),),eq,20).
sum(((1,(time,1)),),geq,20) :- sum(((1,(time,1)),),eq,20).
sum(((1,(time,2)),),geq,20) :- sum(((1,(time,2)),),eq,20).
sum(((1,(time,3)),),geq,20) :- sum(((1,(time,3)),),eq,20).
sum(((1,(time,0)),),geq,30) :- sum(((1,(time,0)),),eq,30).
sum(((1,(time,1)),),geq,30) :- sum(((1,(time,1)),),eq,30).
sum(((1,(time,2)),),geq,30) :- sum(((1,(time,2)),),eq,30).
sum(((1,(time,3)),),geq,30) :- sum(((1,(time,3)),),eq,30).
sum(((1,(time,4)),),geq,30) :- sum(((1,(time,4)),),eq,30).
sum(((1,(time,0)),),geq,40) :- sum(((1,(time,0)),),eq,40).
sum(((1,(time,1)),),geq,40) :- sum(((1,(time,1)),),eq,40).
sum(((1,(time,2)),),geq,40) :- sum(((1,(time,2)),),eq,40).
sum(((1,(time,3)),),geq,40) :- sum(((1,(time,3)),),eq,40).
sum(((1,(time,4)),),geq,40) :- sum(((1,(time,4)),),eq,40).
sum(((1,(time,0)),),geq,10) :- sum(((1,(time,0)),),gt,10).
sum(((1,(time,1)),),geq,10) :- sum(((1,(time,1)),),gt,10).
sum(((1,(time,2)),),geq,10) :- sum(((1,(time,2)),),gt,10).
sum(((1,(time,3)),),geq,10) :- sum(((1,(time,3)),),gt,10).
sum(((1,(time,0)),),geq,20) :- sum(((1,(time,0)),),gt,20).
sum(((1,(time,1)),),geq,20) :- sum(((1,(time,1)),),gt,20).
sum(((1,(time,2)),),geq,20) :- sum(((1,(time,2)),),gt,20).
sum(((1,(time,3)),),geq,20) :- sum(((1,(time,3)),),gt,20).
sum(((1,(time,0)),),geq,30) :- sum(((1,(time,0)),),gt,30).
sum(((1,(time,1)),),geq,30) :- sum(((1,(time,1)),),gt,30).
sum(((1,(time,2)),),geq,30) :- sum(((1,(time,2)),),gt,30).
sum(((1,(time,3)),),geq,30) :- sum(((1,(time,3)),),gt,30).
sum(((1,(time,4)),),geq,30) :- sum(((1,(time,4)),),gt,30).
sum(((1,(time,0)),),geq,40) :- sum(((1,(time,0)),),gt,40).
sum(((1,(time,1)),),geq,40) :- sum(((1,(time,1)),),gt,40).
sum(((1,(time,2)),),geq,40) :- sum(((1,(time,2)),),gt,40).
sum(((1,(time,3)),),geq,40) :- sum(((1,(time,3)),),gt,40).
sum(((1,(time,4)),),geq,40) :- sum(((1,(time,4)),),gt,40).
&sum { (time,1) } > (time,(1-1)).
&sum { (time,2) } > (time,(2-1)).
&sum { (time,3) } > (time,(3-1)).
 :- not map(10,*).
 :- not map(20,*).
 :- not map(30,*).
 :- not map(40,*).
&sum { (time,4) } = 1000.
significant_step(1) :- happens(increment,1).
significant_step(2) :- happens(increment,2).
significant_step(3) :- happens(increment,3).
significant_step(1) :- happens(reset,1).
significant_step(2) :- happens(reset,2).
significant_step(3) :- happens(reset,3).
 :- not significant_step(1).
 :- not significant_step(2).
 :- not significant_step(3).
 :- releasedAt(*,0).
