#pragma once

#include <ios>
#include <iostream>
#include <string>
#include <string_view>

template <typename T, typename Functor>
T Prompt(std::string_view message, Functor verify) {
  while (true) {
    std::cout << message << " ";
    T value;
    std::cin >> value;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (!std::cin.good()) {
      if (std::cin.eof()) std::exit(0);
      std::cout << "Invalid input.\n";
    } else if (verify(value)) {
      return value;
    }
    std::cin.clear();
  }
}
