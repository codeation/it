ARG dev_gcc_version

FROM gcc:$dev_gcc_version

RUN apt-get update \
 && apt-get install -y apt-utils \
 && apt-get install -y clang-format \
 && apt-get install -y libgtk-3-dev \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*
