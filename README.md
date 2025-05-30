# grug benchmarks

See [my blog post](https://mynameistrez.github.io/2024/02/29/creating-the-perfect-modding-language.html) for an introduction to the grug modding language.

## Running

1. Clone this repository and open it in VS Code.
2. Run `git submodule update --init` to clone the `grug.c` and `grug.h` files (for your own game you can just drop these files directly into your project).
3. Hit F5 to run the program.

## Profiling with perf on Linux

1. Run `perf record -g ./a.out`.

If you get `Error: Access to performance monitoring and observability operations is limited.`, you can read the instructions.

You can make a temporary change using `sudo sysctl kernel.perf_event_paranoid=-1`. To make it permanent I ran `sudo nano /etc/sysctl.conf`, added `kernel.perf_event_paranoid = -1` at the end of the file, and restarted my computer.

If you see the warning `Kernel address maps are restricted. ... Check /proc/sys/kernel/kptr_restrict`, then you should see that running `cat /proc/sys/kernel/kptr_restrict` prints 1. If so, run `sudo nano /etc/sysctl.conf`, and add `kernel.kptr_restrict = 0`.

2. Run `perf report`, select `test_1M_cached`, and then select `Annotate test_1M_cached` to see which lines of code took the longest to run.

### Visualizing the stack trace with FlameGraph

Using FlameGraph, I got this output:

![image](https://github.com/user-attachments/assets/e9c0647c-36b9-432d-b1b5-4d7b33216605)

![image](https://github.com/user-attachments/assets/c1bba5af-9b11-4f13-bd2e-65a4fe42258a)

What was slow here was the fact that every `on_` function call enabled and disabled signal runtime error handling (division by 0/stack overflow/etc.).

Much later on I was able to optimize it down to this, by replacing signal handler usage with manual checks in the Assembly (like preventing division by 0, by checking if the divisor is 0 beforehand):

![image](https://github.com/user-attachments/assets/92e56bfc-ee84-4eb9-b8d8-6a69485030ea)

![image](https://github.com/user-attachments/assets/8770e144-5798-49ef-8eb4-33e1add59687)

I decided to call this behavior "safe" mode, and I then modified the compiler so that it also generates a "fast" version of every `on_` function. The "fast" mode *does not* protect against mod runtime errors:

![image](https://github.com/user-attachments/assets/2af7dc83-6bd8-4a3e-9829-d7f73d7acf7b)

After making the 1st and 2nd test do 1000x more iterations, and the 3rd test 100x more iterations:

![image](https://github.com/user-attachments/assets/008e5160-ed28-4b9d-947d-0fab21d96e03)

![image](https://github.com/user-attachments/assets/2224a3b1-df14-42e5-8efc-7cb812953767)

Now that the `get_1()` and `increment()` calls are pretty much all the graph shows, there isn't much left to optimize (without writing an entire optimization pass for the generated machine code):

![image](https://github.com/user-attachments/assets/6f7e9634-f8c1-4761-9f33-c1e874b27730)

![image](https://github.com/user-attachments/assets/4c9488fc-10b8-4115-9451-977eaa0812a9)

#### Steps

1. Clone [FlameGraph](https://github.com/brendangregg/FlameGraph).
2. After you've generated `perf.data` using the earlier `perf record -g ./a.out`, run `sudo perf script -f > out.perf` to output the trace to `out.perf`.
3. Run `pwd` in your FlameGraph clone, add `export FLAMEGRAPH_PATH=<pwd path goes here>` to your `~/.bashrc`, and run `source ~/.bashrc` to apply the change to your current terminal session.
4. Run `$FLAMEGRAPH_PATH/stackcollapse-perf.pl out.perf > out.folded`.
5. Run `$FLAMEGRAPH_PATH/flamegraph.pl out.folded --width 2450 --fontsize 15 > kernel.svg`.
6. Drag the generated `kernel.svg` into your browser as a new tab. You're able to click the colored boxes to look at a subsection of the stack trace.
