FROM alpine:3.22

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=${USER_UID}

ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
ENV SHELL=/bin/zsh

RUN apk add --no-cache \
        bash \
        build-base \
        ca-certificates \
        can-utils \
        cmake \
        cppcheck \
        curl \
        doxygen \
        gdb \
        git \
        gtest-dev \
        graphviz \
        iproute2 \
        kmod \
        linux-headers \
        make \
        nano \
        ninja \
        openssh-client \
        pkgconf \
        shadow \
        sudo \
        zsh \
    && addgroup -g "${USER_GID}" "${USERNAME}" \
    && adduser -D -u "${USER_UID}" -G "${USERNAME}" -s /bin/zsh "${USERNAME}" \
    && addgroup "${USERNAME}" wheel \
    && echo "%wheel ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/wheel \
    && chmod 0440 /etc/sudoers.d/wheel \
    && git clone --depth=1 https://github.com/ohmyzsh/ohmyzsh.git "/home/${USERNAME}/.oh-my-zsh" \
    && cp "/home/${USERNAME}/.oh-my-zsh/templates/zshrc.zsh-template" "/home/${USERNAME}/.zshrc" \
    && sed -i 's/^ZSH_THEME=.*/ZSH_THEME="tjkirch"/' "/home/${USERNAME}/.zshrc" \
    && chown -R "${USERNAME}:${USERNAME}" "/home/${USERNAME}"

USER ${USERNAME}
WORKDIR /workspaces/libsockcanpp

SHELL ["/bin/zsh", "-c"]
