% A larger instance with two resources and several conflicts.
item(i1, 28, 2).
requires(i1, weight, 5). requires(i1, volume, 11).
item(i2, 5, 1).
requires(i2, weight, 11). requires(i2, volume, 9).
item(i3, 40, 3).
requires(i3, weight, 15). requires(i3, volume, 9).
item(i4, 39, 4).
requires(i4, weight, 5). requires(i4, volume, 5).
item(i5, 30, 4).
requires(i5, weight, 15). requires(i5, volume, 10).
item(i6, 14, 2).
requires(i6, weight, 12). requires(i6, volume, 4).
item(i7, 5, 4).
requires(i7, weight, 15). requires(i7, volume, 10).
item(i8, 7, 2).
requires(i8, weight, 12). requires(i8, volume, 3).
item(i9, 35, 3).
requires(i9, weight, 6). requires(i9, volume, 2).
item(i10, 30, 4).
requires(i10, weight, 11). requires(i10, volume, 8).
item(i11, 13, 4).
requires(i11, weight, 13). requires(i11, volume, 11).
item(i12, 13, 1).
requires(i12, weight, 7). requires(i12, volume, 3).
item(i13, 32, 3).
requires(i13, weight, 9). requires(i13, volume, 5).
item(i14, 31, 3).
requires(i14, weight, 14). requires(i14, volume, 12).
item(i15, 39, 3).
requires(i15, weight, 10). requires(i15, volume, 8).
item(i16, 26, 2).
requires(i16, weight, 11). requires(i16, volume, 8).
item(i17, 15, 3).
requires(i17, weight, 12). requires(i17, volume, 2).
item(i18, 18, 1).
requires(i18, weight, 13). requires(i18, volume, 7).
item(i19, 23, 3).
requires(i19, weight, 12). requires(i19, volume, 11).
item(i20, 35, 4).
requires(i20, weight, 3). requires(i20, volume, 3).

capacity(weight, 60).
capacity(volume, 45).

conflict(i1, i5).
conflict(i2, i20).
conflict(i3, i12).
conflict(i3, i14).
conflict(i4, i14).
conflict(i10, i14).
conflict(i11, i18).
conflict(i13, i19).
