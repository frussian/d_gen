//
// Created by Anton on 25.05.2023.
//

#include "BuildError.h"

BuildError::BuildError(std::vector<Err> errors): errors(std::move(errors)) {}
