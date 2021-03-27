"""Dependency specific initialization for libprocess."""

load("@com_github_3rdparty_bazel_rules_http_parser//bazel:deps.bzl", http_parser="deps")
load("@com_github_3rdparty_bazel_rules_libevent//bazel:deps.bzl", libevent="deps")
load("@com_github_3rdparty_stout//bazel:deps.bzl", stout="deps")

def deps(**kwargs):
    http_parser(**kwargs)
    libevent(**kwargs)
    stout(**kwargs)
