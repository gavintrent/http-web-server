### Build/test container ###
# Define builder stage
FROM name-not-found-404:base AS builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Configure, build, test, and print the coverage summary
RUN cmake -DCMAKE_BUILD_TYPE=Coverage .. \
    && make coverage
