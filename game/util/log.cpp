/* Creation Date: 16/7/2023
 * Creator: obwan02
 * ========================
 *
 * This file contains code for
 * logging functions
 */
#include "log.h"

namespace wrld::log {

void log(String str) {
  write(0, str.data, str.len);
}

void logln(String str) {
  write(0, str.data, str.len);
  write(0, "\n", 1);
}

}; // namespace wrld::log
