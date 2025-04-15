### Build/test container ###
# Define builder stage
FROM name-not-found-404:base AS builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:noble AS deploy

# Copy server output binary to "."
COPY --from=builder /usr/src/project/build/bin/server .
COPY --from=builder /usr/src/project/config/default.config .

# Expose some port(s)
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
# Update with real server name
ENTRYPOINT ["./server"]

# Use CMD to specify arguments to ENTRYPOINT
# Update with real server args
CMD ["gcloud.config"]
