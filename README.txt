jdcruz1 John D'cruz
yli302 Yiyang Li (Sky)

In calc.cpp, we added a mutex to protect our Calc data. We determined that the calc_eval function, specifically the call to evalExpr, was the critical section because it was the part of the code that actually used the data across multiple sessions, and therefore that data needed to be locked and locked before and after the call. In calcServer.cpp, we then added all necessary handling for pthread, including the worker function.
