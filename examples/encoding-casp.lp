task(T) :- executionTime(T,_,_).
machine(M) :- executionTime(_,M,_).

% decide which operation first
{seq((T,ST1),(T,ST2),Time1)} :- assign(T,ST1,M1), assign(T,ST2,M2), ST1<ST2, executionTime(T,ST1,Time1), executionTime(T,ST2,Time2).
seq((T,ST2),(T,ST1),Time2) :- assign(T,ST1,M1), assign(T,ST2,M2), ST1<ST2, executionTime(T,ST1,Time1), executionTime(T,ST2,Time2), not seq((T,ST1),(T,ST2),Time1).

% decide which task first on machine
{seq((T1,ST1),(T2,ST2),Time1)} :- assign(T1,ST1,M), assign(T2,ST2,M), T1<T2, executionTime(T1,ST1,Time1), executionTime(T2,ST2,Time2).
seq((T2,ST2),(T1,ST1),Time2) :- assign(T1,ST1,M), assign(T2,ST2,M), T1<T2, executionTime(T1,ST1,Time1), executionTime(T2,ST2,Time2), not seq((T1,ST1),(T2,ST2),Time1).

&sum { start(T1,M1); -start(T2,M2) } <= -Time :- seq((T1,M1),(T2,M2),Time).

&dom { 0..N } = start(T,M) :- task(T), executionTime(T,M,Time), bound(N).

#show seq/3.
