@echo off
rem
rem Renaming all .cpp files back to .cc
rem

for %%i in (aloha cqn dyna fddi fifo hcube hist neddemo p-to-p queueing routing queues tictoc tokenring) do ren %%i\*.cpp *.cc