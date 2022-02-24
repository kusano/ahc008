P1 = [[0.]*21 for _ in range(21)]
P2 = [0.]*21

F = [0]*21
F[0] = 1
for i in range(1, 21):
  F[i] = F[i-1]*i

for N in range(10, 21):
  for d in range(21):
    for c in range(21):
      if d+c<=N:
        p = .2**d * .2**c * .6**(N-d-c) * F[N]/F[d]/F[c]/F[N-d-c] / 11
        P1[d][c] += p
        P2[d+c] += p

for p in P1:
  print(*p)

print(*P2)
