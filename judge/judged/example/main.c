#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 5000

int main() {
  int a, b;
  char msg[10];

  scanf("%s", msg); getchar();
  printf("Hello, %s! (input len=%lu)\n", msg, strlen(msg));
  scanf("%d%d", &a, &b);
  printf("%d\n", a+b);

  int i, j;
  int num[N];
  for(i=0; i<=N; i++) num[i] = rand() % (2 * N);
  for(i=0; i<N-1; i++)
    for(j=i+1; j<N; j++)
      if(num[i] > num[j])
      {
        num[i]^=num[j];
        num[j]^=num[i];
        num[i]^=num[j];
      }
  for(i=0; i<N; i++)
    printf("%d ", num[i]);
  printf("\n");

  char* s = malloc(3 * sizeof(char));
  memset(s, 0, 3 * sizeof(char));
  s[0]='5'; s[1]='0'; s[2]='\0';
  printf("%s\n", s);
  free(s);

  int zero;
  scanf("%d", &zero);
  printf("%d\n", 20 + zero);

  return 0;
}
