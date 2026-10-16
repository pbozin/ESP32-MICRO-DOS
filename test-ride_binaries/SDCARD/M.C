#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <graphics.h>
#include <picoc_esp32.h>

#define MAX_FILES 64
#define MAX_NAME_LEN 32 

int termW, termH;
int canvasH; 
int charW, charH;

char leftFiles[MAX_FILES][MAX_NAME_LEN];
char rightFiles[MAX_FILES][MAX_NAME_LEN];
int leftCount = 0;
int rightCount = 0;

int activePanel = 0; 
int leftIndex = 0;
int rightIndex = 0;

char leftPath[MAX_NAME_LEN] = "/sd/";
char rightPath[MAX_NAME_LEN] = "/sd/";

void clearLeftFiles() {
   for (int i=0; i<MAX_FILES; i++) {
       memset(leftFiles[i], 0, MAX_NAME_LEN);
   }
}

void clearRightFiles() {
   for (int i=0; i<MAX_FILES; i++) {
       memset(rightFiles[i], 0, MAX_NAME_LEN);
   }
}

void read_left_directory() {
    leftCount = 0;
    clearLeftFiles();

    if (strcmp(leftPath, "/sd/") != 0) {
        strcpy(leftFiles[leftCount], "..");
        leftCount++;
    }

    DIR *dir = opendir(leftPath);
    if (dir == NULL) {
        strcpy(leftFiles[leftCount], "EMPTY");
        leftCount++;
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
	char tmpPath[MAX_NAME_LEN];
	strcpy(tmpPath, leftPath);
	strcat(tmpPath, dirent_getname(entry));
        struct stat temp;
	stat(tmpPath, &temp);
        strcpy(leftFiles[leftCount], dirent_getname(entry));
        if (temp.is_dir) {
            strcat(leftFiles[leftCount], "/");
	}
        leftCount++;
    }

    closedir(dir);
}

void read_right_directory() {
    rightCount = 0;
    clearRightFiles();

    if (strcmp(rightPath, "/sd/") != 0) {
        strcpy(rightFiles[rightCount], "..");
        rightCount++;
    }

    DIR *dir = opendir(rightPath);
    if (dir == NULL) {
        strcpy(rightFiles[rightCount], "EMPTY");
        rightCount++;
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
	char tmpPath[MAX_NAME_LEN];
        struct stat temp;
	strcpy(tmpPath, rightPath);
	strcat(tmpPath, dirent_getname(entry));
	stat(tmpPath, &temp);
        strcpy(rightFiles[rightCount], dirent_getname(entry));
        if (temp.is_dir) {
            strcat(rightFiles[rightCount], "/");
	} 
        rightCount++;
    }

    closedir(dir);

}

void draw_interface() {
    int i;
    int panelW = termW / 2;
    int maxLines = (termH / charH) - 3;
    int filesStartY = 2;

    fillRect(0, 0, panelW, (canvasH - 1), BLUE);
    drawRect(0, 0, panelW, (canvasH - 1), CYAN);
    setTextColor(YELLOW);
    printAt(1, 1, leftPath);

    for (i = 0; i <= maxLines; i++) {
	int leftSkip = 0;
	if ( leftIndex >= maxLines ) {
	    leftSkip = (leftIndex-maxLines);
	}
        int color = (activePanel == 0 && leftIndex - leftSkip == i) ? BLACK : WHITE;
        if (activePanel == 0 && leftIndex - leftSkip == i) {
            fillRect(1, (filesStartY + i) * charH - (charH / 4), (panelW - 2), charH, CYAN);
            setBgColor(CYAN);
        }
        setTextColor(color);
        printAt(2, filesStartY + i, leftFiles[i+leftSkip]);
        setBgColor(BLUE);
    }

    fillRect(panelW, 0, panelW, (canvasH - 1), BLUE);
    drawRect(panelW, 0, panelW, (canvasH - 1), CYAN);
    setTextColor(YELLOW);
    printAt((int)(panelW/charW) + 1, 1, rightPath);

    for (i = 0; i <= maxLines; i++) {
	int rightSkip = 0;
	if ( rightIndex >= maxLines ) {
	    rightSkip = (rightIndex-maxLines);
	}
        int color = (activePanel == 1 && rightIndex - rightSkip == i) ? BLACK : WHITE;
        if (activePanel == 1 && rightIndex - rightSkip == i) {
            fillRect((panelW + 1), (filesStartY + i) * charH - (charH / 4), (panelW - 2), charH, CYAN);
            setBgColor(CYAN);
        }
        setTextColor(color);
        printAt((int)(panelW/charW) + 2, filesStartY + i, rightFiles[i+rightSkip]);
        setBgColor(BLUE);
    }
}

void change_directory(char* currentPath, char* targetDir) {
    if (strcmp(targetDir, "..") == 0) {
        int len = strlen(currentPath);
        if (len > 4 && currentPath[len - 1] == '/') {
            currentPath[len - 1] = '\0';
        }
        while (len > 4 && currentPath[len - 1] != '/') {
            currentPath[len - 1] = '\0';
            len--;
        }
    } else if (targetDir[strlen(targetDir)-1] == '/') {
        strcat(currentPath, targetDir);
    } else {
      return;
    }

    if (strlen(currentPath) == 0) {
        strcpy(currentPath, "/sd/");
    }
}

void main() {
    termW  = getTermWidth();
    termH  = getTermHeight();
    charW  = getCharWidth();
    charH  = getCharHeight();

    canvasH = termH;

    setFKeys("  <", "  >", "  v", "  ^", " QUIT");
    int origBgColor = getBgColor();
    int origTextColor = getTextColor();
    setBgColor(BLUE);
    clearScreen();

    strcpy(leftPath, "/sd/");
    strcpy(rightPath, "/sd/");

    read_left_directory();
    read_right_directory();

    char ch;
    int exitFlag = 0;

    while (!exitFlag) {
        draw_interface();
	flushGameMatrix();
        ch = getchar();

        switch(ch) {
            case '\x11':
                activePanel = 0;
                break;
                
            case '\x12':
                activePanel = 1;
                break;
                
            case '\x13':
                if (activePanel == 0) {
                    leftIndex = (leftIndex + 1) % leftCount;
                } else {
                    rightIndex = (rightIndex + 1) % rightCount;
                }
                break;
                
            case '\x14':
                if (activePanel == 0) {
                    leftIndex = (leftIndex - 1 + leftCount) % leftCount;
                } else {
                    rightIndex = (rightIndex - 1 + rightCount) % rightCount;
                }
                break;
                
            case '\n':
                if (activePanel == 0 && leftCount > 0) {
                    change_directory(leftPath, leftFiles[leftIndex]);
                    read_left_directory();
                    leftIndex = 0;
                } else if (activePanel == 1 && rightCount > 0) {
                    change_directory(rightPath, rightFiles[rightIndex]);
                    read_right_directory();
                    rightIndex = 0;
                }
                clearScreen();
                break;

            case '\x15':
                exitFlag = 1;
                break;
        }
        delay(30);
    }

    clearFKeys();
    setTextColor(origTextColor);
    setBgColor(origBgColor);
    clearScreen();
}

