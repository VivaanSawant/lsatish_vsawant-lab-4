/**
 * @file main.c
 *
 * @brief      User-mode servo console. Reads commands from the serial
 *             console and drives the servos through the servo syscalls.
 *
 *             Commands: enable <ch>, disable <ch>, set <ch> <angle>
 *
 * @date       2026-10-26
 *
 * @author     Vivaan Sawant (vsawant@andrew.cmu.edu)
 */

#include <stdio.h>
#include <string.h>

#include <servo.h>

#define UNUSED __attribute__((unused))

/** @brief Max length of one input line */
#define LINE_LEN 64

/** @brief Max length of the command word */
#define CMD_LEN 16

/** @brief Lowest valid servo channel */
#define CHANNEL_MIN 1

/** @brief Highest valid servo channel */
#define CHANNEL_MAX 2

/** @brief Lowest valid servo angle in degrees */
#define ANGLE_MIN 0

/** @brief Highest valid servo angle in degrees */
#define ANGLE_MAX 180

/** @brief Characters treated as whitespace when parsing a line */
#define WHITESPACE " \t\r\n"

/** @brief Prints the welcome banner and command list */
static void print_help(void) {
  printf("Welcome to Servo Controller!\n");
  printf("Commands\n");
  printf("  enable <ch>: Enable servo channel\n");
  printf("  disable <ch>: Disable servo channel\n");
  printf("  set <ch> <angle>: Set servo angle\n");
}

/**
 * @brief Checks that only whitespace follows the first n consumed characters.
 *
 * @param line     input line
 * @param consumed number of characters already parsed
 *
 * @return 1 if the rest of the line is whitespace, 0 otherwise
 */
static int rest_is_blank(const char *line, int consumed) {
  return line[consumed + strspn(line + consumed, WHITESPACE)] == '\0';
}

/**
 * @brief Parses and runs one command line.
 *
 * Channel and angle are range-checked as ints before the call, so values
 * like 257 can't wrap around to a valid uint8_t.
 *
 * @param line null-terminated input line
 *
 * @return 0 if the command ran, -1 if it was invalid
 */
static int run_command(const char *line) {
  char cmd[CMD_LEN];
  int ch = 0;
  int angle = 0;
  int consumed = 0;

  if (sscanf(line, "%15s %d%n", cmd, &ch, &consumed) != 2) return -1;
  if (ch < CHANNEL_MIN || ch > CHANNEL_MAX) return -1;

  if (strcmp(cmd, "enable") == 0 && rest_is_blank(line, consumed)) {
    return servo_enable(ch, 1);
  }
  if (strcmp(cmd, "disable") == 0 && rest_is_blank(line, consumed)) {
    return servo_enable(ch, 0);
  }
  if (strcmp(cmd, "set") == 0) {
    int consumed_angle = 0;
    if (sscanf(line, "%*s %*d %d%n", &angle, &consumed_angle) != 1) return -1;
    if (angle < ANGLE_MIN || angle > ANGLE_MAX) return -1;
    if (!rest_is_blank(line, consumed_angle)) return -1;
    return servo_set(ch, angle);
  }
  return -1;
}

/**
 * @brief Console loop: prompt, read a line, run it, repeat forever.
 *
 * @param argc unused
 * @param argv unused
 *
 * @return never returns
 */
int main(UNUSED int argc, UNUSED char const *argv[]) {
  char line[LINE_LEN];

  print_help();

  while (1) {
    printf("\n> ");
    fflush(stdout);

    if (fgets(line, sizeof(line), stdin) == NULL) {
      clearerr(stdin);   /* EOT: clear the EOF flag and keep going */
      continue;
    }

    if (line[strspn(line, WHITESPACE)] == '\0') continue;   /* blank line */

    if (run_command(line) != 0) {
      printf("Invalid command\n");
    }
  }

  return 0;
}