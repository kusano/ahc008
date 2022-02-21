from pwn import *

context.log_level = "error"

n = 100
sum = 0
for i in range(n):
  p = process(f"cd tools; cargo run --release --bin tester ../a.out < in/{i:04d}.txt > /dev/null", shell=True)
  s = int(p.readall().decode().split("\n")[-2].split()[-1])
  print(i, s)
  sum += s
avg = sum/n
print(avg/1e8)
