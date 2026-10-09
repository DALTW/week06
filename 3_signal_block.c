/*
 * 3_signal_block.c — 시그널을 막았다가 풀어 본다 (원본 3_signal_block.c 변형)
 *
 * [원본과 달라진 점]
 *   - 0/1 플래그 대신 핸들러 실행 횟수(delivered)를 센다
 *     → 막힌 동안 Ctrl+C 를 여러 번 눌러도 풀린 뒤 "1번"만 전달되는 것을 확인
 *   - 막힌 5초를 1초씩 나눠, 매초 sigpending 으로 SIGINT 가 대기 중인지 보여 준다
 *     → 핸들러는 아직 안 돌았지만 시그널은 이미 "와 있다"는 것을 확인
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 3_signal_block 3_signal_block.c
 *   ./3_signal_block      # 막혀 있는 5초 동안 Ctrl+C 를 여러 번 눌러 본다
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define BLOCK_SEC 5

static volatile sig_atomic_t delivered = 0;   /* 핸들러가 실행된 횟수 */

static void handler(int sig)
{
    (void)sig;
    delivered++;
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

    sigset_t block, old, pending;
    sigemptyset(&block);
    sigaddset(&block, SIGINT);

    /* SIG_BLOCK: 지금 마스크에 SIGINT 를 추가로 막는다. 원래 마스크는 old 에 저장 */
    if (sigprocmask(SIG_BLOCK, &block, &old) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("지금부터 %d초간 SIGINT 를 막습니다. Ctrl+C 를 여러 번 눌러 보세요.\n", BLOCK_SEC);
    for (int i = 1; i <= BLOCK_SEC; i++) {
        sleep(1);   /* SIGINT 가 막혀 있으므로 Ctrl+C 를 눌러도 sleep 이 깨지지 않는다 */
        if (sigpending(&pending) == -1) {
            perror("sigpending");
            return 1;
        }
        /* 한글은 바이트 수와 화면 폭이 달라 %-6s 로는 줄이 안 맞으므로 공백을 직접 맞춘다 */
        printf("  [%d/%d초] SIGINT 대기(pending) 중? %s | 핸들러 실행 횟수: %d\n",
               i, BLOCK_SEC,
               sigismember(&pending, SIGINT) ? "예    " : "아니오",
               (int)delivered);
    }

    printf("블록 해제 직전 — 핸들러 실행 횟수: %d\n", (int)delivered);

    /* 원래 마스크로 되돌린다 — 이 순간 대기 중이던 SIGINT 가 전달된다 */
    if (sigprocmask(SIG_SETMASK, &old, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("블록 해제 직후 — 핸들러 실행 횟수: %d\n", (int)delivered);
    if (delivered > 0)
        printf("→ 막힌 동안 누른 Ctrl+C 는 버려지지 않고, 풀린 순간 %d번 전달되었습니다.\n",
               (int)delivered);
    else
        printf("→ 막힌 동안 Ctrl+C 가 오지 않았습니다.\n");
    return 0;
}
