#include <iostream>
#include "duphunter/version.h"

int main() {
  std::cout << "DupHunter " << duphunter::version() << '\n';
  std::cout << "Usage: duphunter <path>\n";
  return 0;
}