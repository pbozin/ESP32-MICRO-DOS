#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <graphics.h>
#include <picoc_esp32.h>

struct Entity {
  int id;
  int x;
  int y;
  int targetX;
  int targetY;
  int dirX;
  int dirY;
  int nextDirX;
  int nextDirY;
  int animOffset;
};

void main() {
  fcntl(0, F_SETFL, O_NONBLOCK);

  setBgColor(0); 
  setTextColor(6); 
  initGameMatrix();
  drawJpeg("/PA.DAT/maze.jpg", 0, 0);

  int* colMap = (int*)malloc(sizeof(int) * 31 * 31);  
  createCollisionMap(4, 4, 
                     getTermWidth()-4,
                     getTermHeight()-4,
                     BLUE, 31, 31, colMap);

  struct Entity g[4];

  g[0].id = createSprite("/PA.DAT/red.spr", 18);
  g[1].id = createSprite("/PA.DAT/orange.spr", 18);
  g[2].id = createSprite("/PA.DAT/cyan.spr", 18);
  g[3].id = createSprite("/PA.DAT/purple.spr", 18);

  g[0].x = 140; g[0].y = 140; 
  g[0].dirX = 0;  g[0].dirY = 1;  g[0].animOffset = 0; 
  
  g[1].x = 150; g[1].y = 140; 
  g[1].dirX = 0; g[1].dirY = -1;  g[1].animOffset = 2;

  g[2].x = 160; g[2].y = 140;
  g[2].dirX = 0;  g[2].dirY = -1; g[2].animOffset = 2;

  g[3].x = 170; g[3].y = 140;
  g[3].dirX = 0;  g[3].dirY = 1;  g[3].animOffset = 0; 

  int i;
  for (i = 0; i < 4; i++) {
    g[i].targetX = g[i].x;
    g[i].targetY = g[i].y;
    g[i].nextDirX = g[i].dirX;
    g[i].nextDirY = g[i].dirY;
  }

  int running = 1;
  int tick = 0;

  int dx[4] = {1, -1, 0, 0};
  int dy[4] = {0, 0, -1, 1};

  while (running) {
    int ch = getchar();

    if (ch == '\x13') {
      running = 0;
    } 

    for (i = 0; i < 4; i++) {
      if (g[i].x == g[i].targetX && g[i].y == g[i].targetY) {
        
        int tileX = g[i].x / 10;
        int tileY = g[i].y / 10;

        int validMoves[4]; 
        int validCount = 0;
        int j;

        for (j = 0; j < 4; j++) {
          if (dx[j] == -g[i].dirX && dy[j] == -g[i].dirY) {
            continue; 
          }

          int tx = tileX + dx[j];
          int ty = tileY + dy[j];

          if (tx >= 0 && tx < 31 && ty >= 0 && ty < 31) {
            if (colMap[ty * 31 + tx] > 0) { 
              validMoves[validCount] = j;
              validCount++;
            }
          }
        }

        if (validCount > 0) {
          int choice = validMoves[rand() % validCount];
          g[i].dirX = dx[choice];
          g[i].dirY = dy[choice];
        } else {
          g[i].dirX = -g[i].dirX;
          g[i].dirY = -g[i].dirY;
        }

        g[i].targetX = g[i].x + (g[i].dirX * 10);
        g[i].targetY = g[i].y + (g[i].dirY * 10);

        if (g[i].dirX > 0)  g[i].animOffset = 6; 
        if (g[i].dirX < 0)  g[i].animOffset = 4; 
        if (g[i].dirY < 0)  g[i].animOffset = 2; 
        if (g[i].dirY > 0)  g[i].animOffset = 0; 
      }

      if (g[i].x != g[i].targetX)
          g[i].x += (g[i].targetX > g[i].x) ? 2 : -2;
      if (g[i].y != g[i].targetY)
          g[i].y += (g[i].targetY > g[i].y) ? 2 : -2;

      int frameCycle = (tick % 4 < 2) ? 0 : 1;
      drawSprite(g[i].id,
		 g[i].x,
		 g[i].y,
		 -1,
                 g[i].animOffset + frameCycle);
    }

    tick++;
    delay(33); 
  }

  free(colMap);

  for (i = 0; i < 4; i++)
      freeSprite(g[i].id);

  closeGameMatrix();

  fcntl(0, F_SETFL, O_RDONLY);
}

