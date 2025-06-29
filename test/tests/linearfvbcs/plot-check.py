import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import numpy.linalg as la

def plot_moose_csv(csvname, uex):
    try:
        # Read the CSV file into a pandas DataFrame
        df = pd.read_csv(csvname)
        
        # Display the first few rows of the DataFrame to verify data loading
        print("DataFrame Head:")
        print(df.head())
        
        # Example: Plotting two columns from the CSV
        # Replace 'Column1' and 'Column2' with the actual names of your columns
        # For a line plot, ensure 'Column1' is suitable for an x-axis (e.g., dates, numerical index)
        plt.figure(figsize=(10, 6)) # Optional: set figure size
        plt.plot(df['x'], df['u'], label = 'FV u') 
        plt.plot(df['x'], uex, label = 'u_ex', linestyle='--') 
        plt.legend(loc="lower right")
        # Add labels and a title for clarity
        plt.xlabel('x')
        plt.ylabel('u')
        plt.title('u vs. x')
        plt.grid(True) # Optional: add a grid
        # Display the plot
        plt.show()

        return [df['u'].to_numpy(), df['x'].to_numpy()]
    except FileNotFoundError:
        print(f"Error: The file '{csv_file_path}' was not found.")
    except KeyError as e:
        print(f"Error: Column not found. Please check your column names. Missing column: {e}")
#	except Exception as e:
#	    print(f"An unexpected error occurred: {e}")

def setup_diff_uex(npts):
  k     = 1.0 # diffusion coeff.
  amp   = 1.0 # sinusoid amplitude, for u_exact
  
  #x_l   = 0.1*np.pi # domain bound (left)
  #x_r   = np.pi     # domain bound (right)
  x_l   = 0.0*np.pi # domain bound (left)
  x_r   = 0.9*np.pi     # domain bound (right)
  x = np.linspace(x_l,x_r,npts)
  u_exact = amp*np.sin(x)
  
  alpha =  5.000 # robin BC coeff for gradient term
  beta  =  2.000 # robin BC coeff for variable term
  #gamma   =  (alpha*amp*np.cos(x_l)) + (beta*amp*np.sin(x_l)) # RHS of Robin BC, applied at left boundary
  gamma   =  (alpha*amp*np.cos(x_r)) + (beta*amp*np.sin(x_r)) # RHS of Robin BC, applied at left boundary
  
  return [u_exact, (alpha, beta, gamma), x ]

def setup_adv_uex(npts):
  c     = 0.1 # advection velocity in x
  amp   = 1.0 # sinusoid amplitude, for u_exact
  
  x_l   = 0.1*np.pi # domain bound (left)
  x_r   = np.pi     # domain bound (right)
  x = np.linspace(x_l,x_r,npts)
  u_exact = amp*np.cos(x)
  
  alpha =  0.001 # robin BC coeff for gradient term
  beta  =  0.999 # robin BC coeff for variable term
  gamma   =  (-alpha*amp*np.sin(x_l)) + (beta*amp*np.cos(x_l)) # RHS of Robin BC, applied at left boundary
   
  return [u_exact, (alpha, beta, gamma), x ]

def plot_uex(uex,x):
    plt.figure(figsize=(10, 6)) # Optional: set figure size
    plt.plot(x, uex, label = 'u_ex', linestyle='--') 
    plt.legend(loc="lower right")
    # Add labels and a title for clarity
    plt.xlabel('x')
    plt.ylabel('u_ex')
    plt.title('u_ex vs. x')
    plt.grid(True) # Optional: add a grid
    # Display the plot
    plt.show()


npts  = 1000

[ uex, (a,b,g),x ] = setup_diff_uex(npts)
#[ uex, (a,b,g),x ] = setup_adv_uex(npts)

csvname = 'diffusion-1d-robin_csv_u_0001.csv' 
#csvname = 'advection-1d-robin_csv_u_0001.csv'

[u, x] = plot_moose_csv(csvname, uex)
#plot_uex(uex,x)

# diagnostics
print(a)
print(b)
print(g)
print(uex[0])
print(uex[-1])
print(u[0])
print(u[-1])
print(la.norm(np.divide(uex,u))/la.norm(uex))
print(la.norm(uex-u)/la.norm(uex))
# print(np.linalg.norm(np.divide(uex,u)))
# print(np.linalg.norm(uex-u))


