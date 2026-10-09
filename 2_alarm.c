/*
 * 2_alarm.c — N초마다 울리는 타이머 (원본 2_alarm.c 변형)
 *
 * [사용법]
 *   ./2_alarm <간격초> <반복횟수>      예) ./2_alarm 2 5
 *
 * [원본과 달라진 점]
 *   - 3초 한 번짜리 타임아웃 대신, 명령행 인자로 받은 간격·횟수만큼 반복해 울린다
 *   - alarm 은 한 번만 울리므로 핸들러가 남은 횟수를 줄이고 다시 alarm(간격) 을 건다
 *     (alarm 은 async-signal-safe 함수라 핸들러에서 불러도 된다 — man 7 signal-safety)
 *   - 핸들러와 공유하는 값은 모두 volatile sig_atomic_t 로 둔다
 *   - 출력은 main 에서 한다
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 2_alarm 2_alarm.c
 *   ./2_alarm 2 5
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t interval_sec = 0;   /* 알람 간격(초) */
static volatile sig_atomic_t remaining = 0;      /* 앞으로 울려야 할 횟수 */

static void on_alarm(int sig)
{
    (void)sig;
    remaining--;
    if (remaining > 0)
        alarm((unsigned int)interval_sec);   /* 다음 알람을 다시 예약한다 */
}

/* 문자열을 1~max 사이 정수로 바꾼다. 실패하면 -1 */
static int parse_int(const char *s, int max)
{
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v < 1 || v > max)
        return -1;
    return (int)v;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "사용법: %s <간격초> <반복횟수>   예) %s 2 5\n", argv[0], argv[0]);
        return 1;
    }

    int interval = parse_int(argv[1], 3600);
    int repeat = parse_int(argv[2], 1000);
    if (interval < 0 || repeat < 0) {
        fprintf(stderr, "간격은 1~3600, 반복횟수는 1~1000 사이 정수여야 합니다\n");
        return 1;
    }

    struct sigaction sa;
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    interval_sec = interval;
    remaining = repeat;

    printf("%d초 간격으로 %d번 울립니다 (PID %d)\n", interval, repeat, getpid());
    alarm((unsigned int)interval);   /* 첫 알람 예약 */

    int done = 0;   /* main 이 출력한 횟수 */
    while (done < repeat) {
        pause();
        while (done < repeat - remaining) {   /* 울린 횟수 = 전체 - 남은 횟수 */
            done++;
            printf("[%d/%d] %d초 경과\n", done, repeat, done * interval);
        }
    }

    alarm(0);   /* 혹시 남은 예약이 있으면 취소한다 */
    printf("타이머 종료\n");
    return 0;
}
