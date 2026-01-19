ffluent(value).
initiallyP(value,0).
obs(happens(increment),10).
obs(happens(increment),20).
obs(happens(reset),30).
obs(happens(increment),40).
significant_step(0).
significant_step(4).
step(0).
step(1).
step(2).
step(3).
step(4).
{ releasedAt(value,0) }.
{ releasedAt(value,1) }.
{ releasedAt(value,2) }.
{ releasedAt(value,3) }.
{ releasedAt(value,4) }.
 :- releasedAt(value,1); not releasedAt(value,0).
 :- releasedAt(value,2); not releasedAt(value,1).
 :- releasedAt(value,3); not releasedAt(value,2).
 :- releasedAt(value,4); not releasedAt(value,3).
comparison(((1,(time,0)),),10).
comparison(((1,(time,1)),),10).
comparison(((1,(time,2)),),10).
comparison(((1,(time,3)),),10).
comparison(((1,(time,4)),),10).
comparison(((1,(time,0)),),20).
comparison(((1,(time,1)),),20).
comparison(((1,(time,2)),),20).
comparison(((1,(time,3)),),20).
comparison(((1,(time,4)),),20).
comparison(((1,(time,0)),),30).
comparison(((1,(time,1)),),30).
comparison(((1,(time,2)),),30).
comparison(((1,(time,3)),),30).
comparison(((1,(time,4)),),30).
comparison(((1,(time,0)),),40).
comparison(((1,(time,1)),),40).
comparison(((1,(time,2)),),40).
comparison(((1,(time,3)),),40).
comparison(((1,(time,4)),),40).
#sum+ { 1,0,sum(((1,(time,0)),),lt,10): sum(((1,(time,0)),),lt,10); 1,0,sum(((1,(time,0)),),eq,10): sum(((1,(time,0)),),eq,10); 1,0,sum(((1,(time,0)),),gt,10): sum(((1,(time,0)),),gt,10) } = 1.
#sum+ { 1,0,sum(((1,(time,1)),),lt,10): sum(((1,(time,1)),),lt,10); 1,0,sum(((1,(time,1)),),eq,10): sum(((1,(time,1)),),eq,10); 1,0,sum(((1,(time,1)),),gt,10): sum(((1,(time,1)),),gt,10) } = 1.
#sum+ { 1,0,sum(((1,(time,2)),),lt,10): sum(((1,(time,2)),),lt,10); 1,0,sum(((1,(time,2)),),eq,10): sum(((1,(time,2)),),eq,10); 1,0,sum(((1,(time,2)),),gt,10): sum(((1,(time,2)),),gt,10) } = 1.
#sum+ { 1,0,sum(((1,(time,3)),),lt,10): sum(((1,(time,3)),),lt,10); 1,0,sum(((1,(time,3)),),eq,10): sum(((1,(time,3)),),eq,10); 1,0,sum(((1,(time,3)),),gt,10): sum(((1,(time,3)),),gt,10) } = 1.
#sum+ { 1,0,sum(((1,(time,4)),),lt,10): sum(((1,(time,4)),),lt,10); 1,0,sum(((1,(time,4)),),eq,10): sum(((1,(time,4)),),eq,10); 1,0,sum(((1,(time,4)),),gt,10): sum(((1,(time,4)),),gt,10) } = 1.
#sum+ { 1,0,sum(((1,(time,0)),),lt,20): sum(((1,(time,0)),),lt,20); 1,0,sum(((1,(time,0)),),eq,20): sum(((1,(time,0)),),eq,20); 1,0,sum(((1,(time,0)),),gt,20): sum(((1,(time,0)),),gt,20) } = 1.
#sum+ { 1,0,sum(((1,(time,1)),),lt,20): sum(((1,(time,1)),),lt,20); 1,0,sum(((1,(time,1)),),eq,20): sum(((1,(time,1)),),eq,20); 1,0,sum(((1,(time,1)),),gt,20): sum(((1,(time,1)),),gt,20) } = 1.
#sum+ { 1,0,sum(((1,(time,2)),),lt,20): sum(((1,(time,2)),),lt,20); 1,0,sum(((1,(time,2)),),eq,20): sum(((1,(time,2)),),eq,20); 1,0,sum(((1,(time,2)),),gt,20): sum(((1,(time,2)),),gt,20) } = 1.
#sum+ { 1,0,sum(((1,(time,3)),),lt,20): sum(((1,(time,3)),),lt,20); 1,0,sum(((1,(time,3)),),eq,20): sum(((1,(time,3)),),eq,20); 1,0,sum(((1,(time,3)),),gt,20): sum(((1,(time,3)),),gt,20) } = 1.
#sum+ { 1,0,sum(((1,(time,4)),),lt,20): sum(((1,(time,4)),),lt,20); 1,0,sum(((1,(time,4)),),eq,20): sum(((1,(time,4)),),eq,20); 1,0,sum(((1,(time,4)),),gt,20): sum(((1,(time,4)),),gt,20) } = 1.
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
map(10,2) :- sum(((1,(time,2)),),eq,10).
map(10,3) :- sum(((1,(time,3)),),eq,10).
map(10,4) :- sum(((1,(time,4)),),eq,10).
map(20,0) :- sum(((1,(time,0)),),eq,20).
map(20,1) :- sum(((1,(time,1)),),eq,20).
map(20,2) :- sum(((1,(time,2)),),eq,20).
map(20,3) :- sum(((1,(time,3)),),eq,20).
map(20,4) :- sum(((1,(time,4)),),eq,20).
map(30,0) :- sum(((1,(time,0)),),eq,30).
map(30,1) :- sum(((1,(time,1)),),eq,30).
map(30,2) :- sum(((1,(time,2)),),eq,30).
map(30,3) :- sum(((1,(time,3)),),eq,30).
map(30,4) :- sum(((1,(time,4)),),eq,30).
map(40,0) :- sum(((1,(time,0)),),eq,40).
map(40,1) :- sum(((1,(time,1)),),eq,40).
map(40,2) :- sum(((1,(time,2)),),eq,40).
map(40,3) :- sum(((1,(time,3)),),eq,40).
map(40,4) :- sum(((1,(time,4)),),eq,40).
happens(increment,0) :- map(10,0).
happens(increment,1) :- map(10,1).
happens(increment,2) :- map(10,2).
happens(increment,3) :- map(10,3).
happens(increment,4) :- map(10,4).
happens(increment,0) :- map(20,0).
happens(increment,1) :- map(20,1).
happens(increment,2) :- map(20,2).
happens(increment,3) :- map(20,3).
happens(increment,4) :- map(20,4).
happens(reset,0) :- map(30,0).
happens(reset,1) :- map(30,1).
happens(reset,2) :- map(30,2).
happens(reset,3) :- map(30,3).
happens(reset,4) :- map(30,4).
happens(increment,0) :- map(40,0).
happens(increment,1) :- map(40,1).
happens(increment,2) :- map(40,2).
happens(increment,3) :- map(40,3).
happens(increment,4) :- map(40,4).
initiates(increment,value,((1,1),(1,(value,0))),0).
initiates(increment,value,((1,1),(1,(value,1))),1).
initiates(increment,value,((1,1),(1,(value,2))),2).
initiates(increment,value,((1,1),(1,(value,3))),3).
initiates(increment,value,((1,1),(1,(value,4))),4).
initiates(reset,value,((0,0),),0).
initiates(reset,value,((0,0),),1).
initiates(reset,value,((0,0),),2).
initiates(reset,value,((0,0),),3).
initiates(reset,value,((0,0),),4).
initiated1(value,0) :- happens(increment,0).
initiated1(value,1) :- happens(increment,1).
initiated1(value,2) :- happens(increment,2).
initiated1(value,3) :- happens(increment,3).
initiated1(value,4) :- happens(increment,4).
initiated1(value,0) :- happens(reset,0).
initiated1(value,1) :- happens(reset,1).
initiated1(value,2) :- happens(reset,2).
initiated1(value,3) :- happens(reset,3).
initiated1(value,4) :- happens(reset,4).
&sum { (value,0) } = (value,(0+1)) :- not releasedAt(value,1); not initiated1(value,0).
&sum { (value,1) } = (value,(1+1)) :- not releasedAt(value,2); not initiated1(value,1).
&sum { (value,2) } = (value,(2+1)) :- not releasedAt(value,3); not initiated1(value,2).
&sum { (value,3) } = (value,(3+1)) :- not releasedAt(value,4); not initiated1(value,3).
&sum { (1*1); (1*(value,0)) } = (value,(0+1)) :- happens(increment,0).
&sum { (1*1); (1*(value,1)) } = (value,(1+1)) :- happens(increment,1).
&sum { (1*1); (1*(value,2)) } = (value,(2+1)) :- happens(increment,2).
&sum { (1*1); (1*(value,3)) } = (value,(3+1)) :- happens(increment,3).
&sum { (0*0) } = (value,(0+1)) :- happens(reset,0).
&sum { (0*0) } = (value,(1+1)) :- happens(reset,1).
&sum { (0*0) } = (value,(2+1)) :- happens(reset,2).
&sum { (0*0) } = (value,(3+1)) :- happens(reset,3).
&sum { (1*(time,0)) } < 10 :- sum(((1,(time,0)),),lt,10).
&sum { (1*(time,1)) } < 10 :- sum(((1,(time,1)),),lt,10).
&sum { (1*(time,2)) } < 10 :- sum(((1,(time,2)),),lt,10).
&sum { (1*(time,3)) } < 10 :- sum(((1,(time,3)),),lt,10).
&sum { (1*(time,4)) } < 10 :- sum(((1,(time,4)),),lt,10).
&sum { (1*(time,0)) } < 20 :- sum(((1,(time,0)),),lt,20).
&sum { (1*(time,1)) } < 20 :- sum(((1,(time,1)),),lt,20).
&sum { (1*(time,2)) } < 20 :- sum(((1,(time,2)),),lt,20).
&sum { (1*(time,3)) } < 20 :- sum(((1,(time,3)),),lt,20).
&sum { (1*(time,4)) } < 20 :- sum(((1,(time,4)),),lt,20).
&sum { (1*(time,0)) } < 30 :- sum(((1,(time,0)),),lt,30).
&sum { (1*(time,1)) } < 30 :- sum(((1,(time,1)),),lt,30).
&sum { (1*(time,2)) } < 30 :- sum(((1,(time,2)),),lt,30).
&sum { (1*(time,3)) } < 30 :- sum(((1,(time,3)),),lt,30).
&sum { (1*(time,4)) } < 30 :- sum(((1,(time,4)),),lt,30).
&sum { (1*(time,0)) } < 40 :- sum(((1,(time,0)),),lt,40).
&sum { (1*(time,1)) } < 40 :- sum(((1,(time,1)),),lt,40).
&sum { (1*(time,2)) } < 40 :- sum(((1,(time,2)),),lt,40).
&sum { (1*(time,3)) } < 40 :- sum(((1,(time,3)),),lt,40).
&sum { (1*(time,4)) } < 40 :- sum(((1,(time,4)),),lt,40).
&sum { (1*(time,0)) } = 10 :- sum(((1,(time,0)),),eq,10).
&sum { (1*(time,1)) } = 10 :- sum(((1,(time,1)),),eq,10).
&sum { (1*(time,2)) } = 10 :- sum(((1,(time,2)),),eq,10).
&sum { (1*(time,3)) } = 10 :- sum(((1,(time,3)),),eq,10).
&sum { (1*(time,4)) } = 10 :- sum(((1,(time,4)),),eq,10).
&sum { (1*(time,0)) } = 20 :- sum(((1,(time,0)),),eq,20).
&sum { (1*(time,1)) } = 20 :- sum(((1,(time,1)),),eq,20).
&sum { (1*(time,2)) } = 20 :- sum(((1,(time,2)),),eq,20).
&sum { (1*(time,3)) } = 20 :- sum(((1,(time,3)),),eq,20).
&sum { (1*(time,4)) } = 20 :- sum(((1,(time,4)),),eq,20).
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
&sum { (1*(time,0)) } > 10 :- sum(((1,(time,0)),),gt,10).
&sum { (1*(time,1)) } > 10 :- sum(((1,(time,1)),),gt,10).
&sum { (1*(time,2)) } > 10 :- sum(((1,(time,2)),),gt,10).
&sum { (1*(time,3)) } > 10 :- sum(((1,(time,3)),),gt,10).
&sum { (1*(time,4)) } > 10 :- sum(((1,(time,4)),),gt,10).
&sum { (1*(time,0)) } > 20 :- sum(((1,(time,0)),),gt,20).
&sum { (1*(time,1)) } > 20 :- sum(((1,(time,1)),),gt,20).
&sum { (1*(time,2)) } > 20 :- sum(((1,(time,2)),),gt,20).
&sum { (1*(time,3)) } > 20 :- sum(((1,(time,3)),),gt,20).
&sum { (1*(time,4)) } > 20 :- sum(((1,(time,4)),),gt,20).
&sum { (1*(time,0)) } > 30 :- sum(((1,(time,0)),),gt,30).
&sum { (1*(time,1)) } > 30 :- sum(((1,(time,1)),),gt,30).
&sum { (1*(time,2)) } > 30 :- sum(((1,(time,2)),),gt,30).
&sum { (1*(time,3)) } > 30 :- sum(((1,(time,3)),),gt,30).
&sum { (1*(time,4)) } > 30 :- sum(((1,(time,4)),),gt,30).
&sum { (1*(time,0)) } > 40 :- sum(((1,(time,0)),),gt,40).
&sum { (1*(time,1)) } > 40 :- sum(((1,(time,1)),),gt,40).
&sum { (1*(time,2)) } > 40 :- sum(((1,(time,2)),),gt,40).
&sum { (1*(time,3)) } > 40 :- sum(((1,(time,3)),),gt,40).
&sum { (1*(time,4)) } > 40 :- sum(((1,(time,4)),),gt,40).
sum(((1,(time,0)),),leq,10) :- sum(((1,(time,0)),),lt,10).
sum(((1,(time,1)),),leq,10) :- sum(((1,(time,1)),),lt,10).
sum(((1,(time,2)),),leq,10) :- sum(((1,(time,2)),),lt,10).
sum(((1,(time,3)),),leq,10) :- sum(((1,(time,3)),),lt,10).
sum(((1,(time,4)),),leq,10) :- sum(((1,(time,4)),),lt,10).
sum(((1,(time,0)),),leq,20) :- sum(((1,(time,0)),),lt,20).
sum(((1,(time,1)),),leq,20) :- sum(((1,(time,1)),),lt,20).
sum(((1,(time,2)),),leq,20) :- sum(((1,(time,2)),),lt,20).
sum(((1,(time,3)),),leq,20) :- sum(((1,(time,3)),),lt,20).
sum(((1,(time,4)),),leq,20) :- sum(((1,(time,4)),),lt,20).
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
sum(((1,(time,4)),),leq,10) :- sum(((1,(time,4)),),eq,10).
sum(((1,(time,0)),),leq,20) :- sum(((1,(time,0)),),eq,20).
sum(((1,(time,1)),),leq,20) :- sum(((1,(time,1)),),eq,20).
sum(((1,(time,2)),),leq,20) :- sum(((1,(time,2)),),eq,20).
sum(((1,(time,3)),),leq,20) :- sum(((1,(time,3)),),eq,20).
sum(((1,(time,4)),),leq,20) :- sum(((1,(time,4)),),eq,20).
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
sum(((1,(time,4)),),geq,10) :- sum(((1,(time,4)),),eq,10).
sum(((1,(time,0)),),geq,20) :- sum(((1,(time,0)),),eq,20).
sum(((1,(time,1)),),geq,20) :- sum(((1,(time,1)),),eq,20).
sum(((1,(time,2)),),geq,20) :- sum(((1,(time,2)),),eq,20).
sum(((1,(time,3)),),geq,20) :- sum(((1,(time,3)),),eq,20).
sum(((1,(time,4)),),geq,20) :- sum(((1,(time,4)),),eq,20).
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
sum(((1,(time,4)),),geq,10) :- sum(((1,(time,4)),),gt,10).
sum(((1,(time,0)),),geq,20) :- sum(((1,(time,0)),),gt,20).
sum(((1,(time,1)),),geq,20) :- sum(((1,(time,1)),),gt,20).
sum(((1,(time,2)),),geq,20) :- sum(((1,(time,2)),),gt,20).
sum(((1,(time,3)),),geq,20) :- sum(((1,(time,3)),),gt,20).
sum(((1,(time,4)),),geq,20) :- sum(((1,(time,4)),),gt,20).
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
&sum { (time,0) } >= 0.
&sum { (time,1) } > (time,(1-1)).
&sum { (time,2) } > (time,(2-1)).
&sum { (time,3) } > (time,(3-1)).
&sum { (time,4) } > (time,(4-1)).
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
#show (((1,(time,0)),),10,20): sum(((1,(time,0)),),eq,10); sum(((1,(time,0)),),eq,20).
#show (((1,(time,0)),),10,30): sum(((1,(time,0)),),eq,10); sum(((1,(time,0)),),eq,30).
#show (((1,(time,0)),),10,40): sum(((1,(time,0)),),eq,10); sum(((1,(time,0)),),eq,40).
#show (((1,(time,1)),),10,20): sum(((1,(time,1)),),eq,10); sum(((1,(time,1)),),eq,20).
#show (((1,(time,1)),),10,30): sum(((1,(time,1)),),eq,10); sum(((1,(time,1)),),eq,30).
#show (((1,(time,1)),),10,40): sum(((1,(time,1)),),eq,10); sum(((1,(time,1)),),eq,40).
#show (((1,(time,2)),),10,20): sum(((1,(time,2)),),eq,10); sum(((1,(time,2)),),eq,20).
#show (((1,(time,2)),),10,30): sum(((1,(time,2)),),eq,10); sum(((1,(time,2)),),eq,30).
#show (((1,(time,2)),),10,40): sum(((1,(time,2)),),eq,10); sum(((1,(time,2)),),eq,40).
#show (((1,(time,3)),),10,20): sum(((1,(time,3)),),eq,10); sum(((1,(time,3)),),eq,20).
#show (((1,(time,3)),),10,30): sum(((1,(time,3)),),eq,10); sum(((1,(time,3)),),eq,30).
#show (((1,(time,3)),),10,40): sum(((1,(time,3)),),eq,10); sum(((1,(time,3)),),eq,40).
#show (((1,(time,4)),),10,20): sum(((1,(time,4)),),eq,10); sum(((1,(time,4)),),eq,20).
#show (((1,(time,4)),),10,30): sum(((1,(time,4)),),eq,10); sum(((1,(time,4)),),eq,30).
#show (((1,(time,4)),),10,40): sum(((1,(time,4)),),eq,10); sum(((1,(time,4)),),eq,40).
#show (((1,(time,0)),),20,30): sum(((1,(time,0)),),eq,20); sum(((1,(time,0)),),eq,30).
#show (((1,(time,0)),),20,40): sum(((1,(time,0)),),eq,20); sum(((1,(time,0)),),eq,40).
#show (((1,(time,1)),),20,30): sum(((1,(time,1)),),eq,20); sum(((1,(time,1)),),eq,30).
#show (((1,(time,1)),),20,40): sum(((1,(time,1)),),eq,20); sum(((1,(time,1)),),eq,40).
#show (((1,(time,2)),),20,30): sum(((1,(time,2)),),eq,20); sum(((1,(time,2)),),eq,30).
#show (((1,(time,2)),),20,40): sum(((1,(time,2)),),eq,20); sum(((1,(time,2)),),eq,40).
#show (((1,(time,3)),),20,30): sum(((1,(time,3)),),eq,20); sum(((1,(time,3)),),eq,30).
#show (((1,(time,3)),),20,40): sum(((1,(time,3)),),eq,20); sum(((1,(time,3)),),eq,40).
#show (((1,(time,4)),),20,30): sum(((1,(time,4)),),eq,20); sum(((1,(time,4)),),eq,30).
#show (((1,(time,4)),),20,40): sum(((1,(time,4)),),eq,20); sum(((1,(time,4)),),eq,40).
#show (((1,(time,0)),),30,40): sum(((1,(time,0)),),eq,30); sum(((1,(time,0)),),eq,40).
#show (((1,(time,1)),),30,40): sum(((1,(time,1)),),eq,30); sum(((1,(time,1)),),eq,40).
#show (((1,(time,2)),),30,40): sum(((1,(time,2)),),eq,30); sum(((1,(time,2)),),eq,40).
#show (((1,(time,3)),),30,40): sum(((1,(time,3)),),eq,30); sum(((1,(time,3)),),eq,40).
#show (((1,(time,4)),),30,40): sum(((1,(time,4)),),eq,30); sum(((1,(time,4)),),eq,40).
 :- releasedAt(*,0).
&sum { (value,0) } = 0.
#show happens/2.
#show obs/2.
#show.
