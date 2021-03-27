"""Adds repostories/archives needed by libprocess."""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

load("//3rdparty/bazel-rules-http-parser:repos.bzl", http_parser="repos")
load("//3rdparty/bazel-rules-libevent:repos.bzl", libevent="repos")
load("//3rdparty/stout:repos.bzl", stout="repos")

def repos(*, external = True, repo_mapping = {}):
    http_parser(repo_mapping = repo_mapping)
    libevent(repo_mapping = repo_mapping)
    stout(repo_mapping = repo_mapping)

    if "com_github_gflags_gflags" not in native.existing_rules():
        http_archive(
            name = "com_github_gflags_gflags",
            urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],
            sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
            strip_prefix = "gflags-2.2.2",
            repo_mapping = repo_mapping,
        )

    if "com_github_google_glog" not in native.existing_rules():
        http_archive(
            name = "com_github_google_glog",
            urls = ["https://github.com/google/glog/archive/d516278b1cd33cd148e8989aec488b6049a4ca0b.zip"],
            sha256 = "62efeb57ff70db9ea2129a16d0f908941e355d09d6d83c9f7b18557c0a7ab59e",
            strip_prefix = "glog-d516278b1cd33cd148e8989aec488b6049a4ca0b",
            repo_mapping = repo_mapping,
        )

    if external and "com_github_3rdparty_libprocess" not in native.existing_rules():
        git_repository(
            name = "com_github_3rdparty_libprocess",
            commit = "7d6f7df757b8394793b9c8b954a9d66b82997ddd",
            remote = "https://github.com/3rdparty/libprocess",
            shallow_since = "1616867519 -0700",
            repo_mapping = repo_mapping,
        )
