# grug benchmarks

See [my blog post](https://mynameistrez.github.io/2024/02/29/creating-the-perfect-modding-language.html) for an introduction to the grug modding language.

## Running

1. Clone this repository and open it in VS Code.
2. Run `git submodule update --init` to clone the `grug.c` and `grug.h` files (for your own game you can just drop these files directly into your project).
3. Hit F5 to run the program.

## Profiling with perf on Linux

1. Run `perf record -g ./a.out`

If you get `Error: Access to performance monitoring and observability operations is limited.`, you can read the instructions.

You can make a temporary change using `sudo sysctl kernel.perf_event_paranoid=-1`. To make it permanent I ran `sudo nano /etc/sysctl.conf`, added `kernel.perf_event_paranoid = -1` at the end of the file, and restarted my computer.

If you see the warning `Kernel address maps are restricted. ... Check /proc/sys/kernel/kptr_restrict`, then you should see that running `cat /proc/sys/kernel/kptr_restrict` prints 1. If so, run `sudo nano /etc/sysctl.conf`, and add `kernel.kptr_restrict = 0`.

2. Run `perf report`, select `test_1M_cached`, and then select `Annotate test_1M_cached` to see which lines of code took the longest to run.
