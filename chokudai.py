"""
from PIL import Image

im = Image.open("chokudai.png")

d = ""
for y in range(30):
  for x in range(30):
    if im.getpixel((x, y))[0]<128:
      d += "#"
    else:
      d += " "
  d += "\n"
print(d)
"""

import sys

chokudai = """
              #####           
           #########          
         ############         
        ##############        
       ######### ######       
      ##########  ######      
     ###########   ######     
     ##########     #####     
    ##########       #####    
    #########        #####    
    ########         #####    
   ########       ########    
   ###########   #########    
   ######       #     ####    
   ##### ####    #### ####    
   #### #      #     # ###    
   ####        ##      ##     
    ###        ##      ##     
     ##     #  ###     #      
      #    #   ##      #      
      #   #           #       
       #  ###     ##  #       
       #  #  #####    #       
        #            #        
        ##           #        
         ##         # #       
         # ##      # ##       
        ##   ###### ####      
      ###        # # ####     
    #####         # # #####   
"""
chokudai = chokudai.split("\n")[1:-1]

N = int(input())
px = [0]*N
py = [0]*N
pt = [0]*N
for i in range(N):
  px[i], py[i], pt[i] = map(int, input().split())
  px[i] -= 1
  py[i] -= 1
  pt[i] -= 1
M = int(input())
hx = [0]*M
hy = [0]*M
for i in range(M):
  hx[i], hy[i] = map(int, input().split())
  hx[i] -= 1
  hy[i] -= 1

state = [0]*5

F = [[False]*300 for x in range(300)]

X = [0, 6, 12, 18, 24, 30]

for turn in range(300):
  move = ""
  for i in range(M):
    if state[i]==0:
      if hy[i]>0:
        move += "L"
        hy[i] -= 1
      elif hx[i]!=X[i]:
        if hx[i]>X[i]:
          move += "U"
          hx[i] -= 1
        else:
          move += "D"
          hx[i] += 1
      else:
        state[i] = 1
    if state[i]==1:
      if (hx[i], hy[i])!=(X[i+1]-1, 0):
        if hx[i]%2==0:
          if hy[i]!=29:
            if hy[i]>0 and chokudai[hx[i]][hy[i]-1]=="#" and not F[hx[i]][hy[i]-1]:
              ok = True
              for j in range(N):
                for dx, dy in [(0, 0), (0, 1), (0, -1), (1, 0), (-1, 0)]:
                  if (hx[i], hy[i]-1)==(px[j]+dx, py[j]+dy):
                    ok = False
              if ok:
                move += "l"
                F[hx[i]][hy[i]-1] = True
              else:
                move += "."
            else:
              move += "R"
              hy[i] += 1
          else:
            move += "D"
            hx[i] += 1
        else:
          if hy[i]!=0:
            if hy[i]<29 and chokudai[hx[i]][hy[i]+1]=="#" and not F[hx[i]][hy[i]+1]:
              ok = True
              for j in range(N):
                for dx, dy in [(0, 0), (0, 1), (0, -1), (1, 0), (-1, 0)]:
                  if (hx[i], hy[i]+1)==(px[j]+dx, py[j]+dy):
                    ok = False
              if ok:
                move += "r"
                F[hx[i]][hy[i]+1] = True
              else:
                move += "."
            else:
              move += "L"
              hy[i] -= 1
          else:
            move += "D"
            hx[i] += 1
      else:
        state[i] = 2
    if state[i]==2:
      move += "."

  print(move)
  sys.stdout.flush()

  move = input().split()
  assert len(move)==N
  for i in range(N):
    for c in move[i]:
      if c=="U":
        px[i] -= 1
      if c=="D":
        px[i] += 1
      if c=="L":
        py[i] -= 1
      if c=="R":
        py[i] += 1

