% Three potential factories supply five shops with trucks carrying 10 units.
factory(north, 120, 40, 2).
factory(south, 100, 35, 3).
factory(east,   80, 25, 4).

demand(a, "12.5"). demand(b, "7.5"). demand(c, "16.8"). demand(d, "6.2"). demand(e, "11.4").

route(north, a, 9). route(north, b, 12). route(north, c, 20). route(north, d, 15). route(north, e, 25).
route(south, a, 18). route(south, b, 10). route(south, c, 8). route(south, d, 14). route(south, e, 12).
route(east,  a, 22). route(east,  b, 16). route(east,  c, 11). route(east,  d, 7).  route(east,  e, 9).

truck(10).
