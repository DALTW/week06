/*
 * 1_sigint.c — Ctrl+C 를 세 번 눌러야 종료한다 (원본 1_sigint.c 변형)
 *
 * [원본과 달라진 점]
 *   - 0/1 플래그(got_sigint) 대신 카운터(sigint_count)로 누른 횟수를 센다
 *   - 핸들러는 카운터만 올린다. 출력(printf)은 전부 main 에서 한다
 *   - 세 번째 Ctrl+C 에서 정리 메시지를 내고 종료한다
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 1_sigint 1_sigint.c
 *   ./1_sigint      # Ctrl+C 를 세 번 눌러 본다
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define NEED_PRESSES 3   /* 종료에 필요한 Ctrl+C 횟수 */

/* 핸들러와 main 이 함께 보는 카운터 — volatile sig_atomic_t 로 선언한다 */
static volatile sig_atomic_t sigint_count = 0;

static void handler(int sig)
{
    (void)sig;
    sigint_count++;   /* 카운터만 올린다. 핸들러 실행 중에는 SIGINT 가 자동으로 블록되므로 겹쳐 실행되지 않는다 */
}

int main(void)
{
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Ctrl+C 를 %d번 눌러야 종료합니다 (PID %d)\n", NEED_PRESSES, getpid());

    int shown = 0;   /* main 이 지금까지 출력한 횟수 */
    while (shown < NEED_PRESSES) {
        pause();     /* 시그널이 올 때까지 잠든다 */

        /* 빠르게 연타해 카운터가 여러 칸 올라갔어도 빠짐없이 출력한다 */
        while (shown < sigint_count && shown < NEED_PRESSES) {
            shown++;
            if (shown < NEED_PRESSES)
                printf("\nCtrl+C %d번째 — %d번 더 누르면 종료합니다\n",
                       shown, NEED_PRESSES - shown);
        }
    }

    printf("\nCtrl+C %d번째 — 정리하고 종료합니다.\n", NEED_PRESSES);
    return 0;
}
