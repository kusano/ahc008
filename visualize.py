from PIL import Image, ImageDraw, ImageFont

chars = Image.open("local/characters.png")
chars = [[chars.crop((x*32, y*32, (x+1)*32, (y+1)*32)) for y in range(2)] for x in range(7)]

font = ImageFont.truetype("local/courbd.ttf", 32)

frame = 0

for seed in range(100):

  fin = open(f"tools/in/{seed:04d}.txt")
  fout = open(f"tools/out/{seed:04d}.txt")

  N = int(fin.readline())
  px = [0]*N
  py = [0]*N
  pt = [0]*N
  for i in range(N):
    px[i], py[i], pt[i] = [int(t)-1 for t in fin.readline().split()]
  M = int(fin.readline())
  hx = [0]*M
  hy = [0]*M
  for i in range(M):
    hx[i], hy[i] = [int(t)-1 for t in fin.readline().split()]
  F = [[False]*30 for _ in range(30)]

  for turn in range(301):
    print(seed, turn)
    if turn>0:
      move = fout.readline()
      for i in range(M):
        if move[i]=="U": hx[i] -= 1
        if move[i]=="D": hx[i] += 1
        if move[i]=="L": hy[i] -= 1
        if move[i]=="R": hy[i] += 1
        if move[i]=="u": F[hx[i]-1][hy[i]] = True
        if move[i]=="d": F[hx[i]+1][hy[i]] = True
        if move[i]=="l": F[hx[i]][hy[i]-1] = True
        if move[i]=="r": F[hx[i]][hy[i]+1] = True
      # ローカルテスターが最後の1ターン分を出してくれない？
      if turn<300:
        moves = fout.readline().split()[1:]
        for i in range(N):
          for m in moves[i]:
            if m=="U": px[i] -= 1
            if m=="D": px[i] += 1
            if m=="L": py[i] -= 1
            if m=="R": py[i] += 1
    # ローカルテスターが最後の1ターン分を出してくれない？
    if turn<300:
      pok = list(map(int, fout.readline().split()[1:]))
      hok = list(map(int, fout.readline().split()[1:]))
      score = int(fout.readline().split()[1])

    im = Image.new("RGB", (1920, 1080), (255, 255, 255, 255))

    draw = ImageDraw.Draw(im)
    draw.text((62,  62), f"Seed:  {seed:12,}", font=font, fill=(192, 192, 192))
    draw.text((60,  60), f"Seed:  {seed:12,}", font=font, fill=(0, 0, 0))
    draw.text((62, 122), f"Turn:  {turn:12,}", font=font, fill=(192, 192, 192))
    draw.text((60, 120), f"Turn:  {turn:12,}", font=font, fill=(0, 0, 0))
    draw.text((62, 182), f"Score: {score:12,}", font=font, fill=(192, 192, 192))
    draw.text((60, 180), f"Score: {score:12,}", font=font, fill=(0, 0, 0))

    for x in range(30):
      for y in range(30):
        draw.rectangle([(480+y*32+1, 60+x*32+1), (480+y*32+30, 60+x*32+30)], fill=(192, 192, 192))

    for i in range(N):
      im.paste(chars[pt[i]][1-pok[i]], (480+py[i]*32, 60+px[i]*32), chars[pt[i]][1-pok[i]])
    for i in range(M):
      im.paste(chars[5][1-hok[i]], (480+hy[i]*32, 60+hx[i]*32), chars[5][1-hok[i]])
    for x in range(30):
      for y in range(30):
        if F[x][y]:
          im.paste(chars[6][0], (480+y*32, 60+x*32), chars[6][0])

    im.save(f"out/out_{frame:06d}.png")
    frame += 1
    if turn==300:
      for _ in range(29):
        im.save(f"out/out_{frame:06d}.png")
        frame += 1

  fin.close()
  fout.close()
