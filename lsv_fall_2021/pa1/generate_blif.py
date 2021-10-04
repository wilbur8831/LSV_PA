from itertools import permutations

perm = permutations(['1', '1', '1', '1', '-', '-'])

lst_perm = list(set(perm))

for i in lst_perm:
    print(''.join(i) + " 1")

