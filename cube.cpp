#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
void usleep(__int64 usec) {
  HANDLE timer;
  LARGE_INTEGER ft;
  ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval
  timer = CreateWaitableTimer(NULL, TRUE, NULL);
  SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
}
#else
#include <unistd.h>
#endif

float A = 0, B = 0, C = 0;

const int width = 160, height = 44;
float zBuffer[width * height];
char buffer[width * height];
const char backgroundASCIICode = '.';
const int distanceFromCam = 100;
const float K1 = 40;
const float incrementSpeed = 0.6;

struct Cube {
  float size;
  float offset;
};

Cube cubes[] = {
    {20, -40},
    {10, 10},
    {5, 40},
};

float calculateX(float i, float j, float k) {
  return j * sin(A) * sin(B) * cos(C) - k * cos(A) * sin(B) * cos(C) +
         j * cos(A) * sin(C) + k * sin(A) * sin(C) + i * cos(B) * cos(C);
}

float calculateY(float i, float j, float k) {
  return j * cos(A) * cos(C) + k * sin(A) * cos(C) -
         j * sin(A) * sin(B) * sin(C) + k * cos(A) * sin(B) * sin(C) -
         i * cos(B) * sin(C);
}

float calculateZ(float i, float j, float k) {
  return k * cos(A) * cos(B) - j * sin(A) * cos(B) + i * sin(B);
}

void calculateForSurface(float cubeX, float cubeY, float cubeZ, int ch,
                         float horizontalOffset) {
  float x = calculateX(cubeX, cubeY, cubeZ);
  float y = calculateY(cubeX, cubeY, cubeZ);
  float z = calculateZ(cubeX, cubeY, cubeZ) + distanceFromCam;

  float ooz = 1 / z;

  int xp = (int)(width / 2 + horizontalOffset + K1 * ooz * x * 2);
  int yp = (int)(height / 2 + K1 * ooz * y);

  int idx = xp + yp * width;
  if (idx >= 0 && idx < width * height) {
    if (ooz > zBuffer[idx]) {
      zBuffer[idx] = ooz;
      buffer[idx] = ch;
    }
  }
}

void renderCubes() {
  memset(buffer, backgroundASCIICode, width * height);
  memset(zBuffer, 0, sizeof(zBuffer));

  for (const Cube &cube : cubes) {
    for (float cubeX = -cube.size; cubeX < cube.size; cubeX += incrementSpeed) {
      for (float cubeY = -cube.size; cubeY < cube.size;
           cubeY += incrementSpeed) {
        calculateForSurface(cubeX, cubeY, -cube.size, '@', cube.offset);
        calculateForSurface(cube.size, cubeY, cubeX, '$', cube.offset);
        calculateForSurface(-cube.size, cubeY, -cubeX, '~', cube.offset);
        calculateForSurface(-cubeX, cubeY, cube.size, '#', cube.offset);
        calculateForSurface(cubeX, -cube.size, -cubeY, ';', cube.offset);
        calculateForSurface(cubeX, cube.size, cubeY, '+', cube.offset);
      }
    }
  }
}

void displayFrame() {
  printf("\x1b[H"); // Reset cursor position
  for (int k = 0; k < width * height; k++) {
    putchar(k % width ? buffer[k] : '\n');
  }
}

int main() {
  printf("\x1b[2J"); // Clear screen
  while (true) {
    renderCubes();
    displayFrame();
    A += 0.04;
    B += 0.04;
    C += 0.01;
    usleep(16000); // 60 FPS
  }
  return 0;
}
