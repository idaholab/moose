from pythonfmu import Fmi2Slave
import socket
from pythonfmu.enums import Fmi2Causality, Fmi2Variability
from pythonfmu.variables import Boolean, Integer, Real, ScalarVariable, String


class SocketSlave(Fmi2Slave):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # Register one input and one output

        self.in_val = 0
        self.out_val = 0

        self.register_variable(Real("in_val", causality=Fmi2Causality.input, variability=Fmi2Variability.continuous))
        self.register_variable(Real("out_val", causality=Fmi2Causality.output, variability=Fmi2Variability.continuous))

    def do_step(self, current_time: float, step_size: float) -> bool:
        try:
          # Connect to local TCP server and exchange data
          with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
              sock.connect(("127.0.0.1", 5000))
              sock.sendall(str(self.in_val).encode())
              resp = sock.recv(1024).decode()

          # Write the received value as the output
          self.out_val=float(resp)
        except ConnectionResetError as e:
            # Optional: attempt to reconnect on reset :contentReference[oaicite:8]{index=8}
            self.sock.close()
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect(('127.0.0.1', 5000))
            raise e

        return True

    def terminate(self):
        try:
            self.sock.close()  # Close the persistent socket :contentReference[oaicite:10]{index=10}
        except Exception:
            pass
        super().terminate()
