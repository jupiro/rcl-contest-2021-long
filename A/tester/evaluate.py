import os

path = "score.txt"
res = 0
cnt = 0
with open(path) as f:
  string = f.readlines()
  for line in string:
    res += float(line.split(':')[1])
    cnt += 1
print(res / 2.0)