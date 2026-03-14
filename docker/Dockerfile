FROM archlinux:latest AS build

RUN bash -c 'yes y | pacman -Sy base-devel cmake git ninja'
COPY  --from=parent . /source
WORKDIR ["/source"]
RUN rm -rf /source/build
RUN ls /source && mkdir -p /source/build && cd /source/build && cmake -G Ninja /source
WORKDIR ["/source/build"]
RUN cmake --build /source/build 

FROM archlinux:latest AS runtime
RUN mkdir /binary && useradd -u 1000 -md /app server
COPY --from=build /source/build /build
COPY ./entrypoint.sh /entrypoint.sh
WORKDIR /app
ENTRYPOINT ["/entrypoint.sh", "/build/vesper-app"]
