
Each step :

Reset grid():
    for each cell in grid :
        set grid type = air
        set grid velocity = 0

particle to grid():
    for each particle
        find its grid position
        set p2g_transfer = fluid
        
        scatter velocity around the particle grid based on the velocity of the particle in the p2g_transfer buffer

apply():
    transfers data from p2g_transfer to grid
    reinitialize the p2g_transfer buffer

apply_Forces():
    for each cell in grid :
        set old_vel to vel
        Add gravity to vel
        add boundary condition forces

setup grid project():
    for each cell in grid:
        ComputeDivergence():
            if cell is fluid
                calculate rhs : difference in velocity from the neigbouring cells
        ComputeA():
            if cell is fluid:
                for each neighbourg
                    if it's fluid:
                        this_cell.a_diag += scale
                        this_cell.a_x -= scale

pressure_solve():
    for all jacobi_iterations
        jacobi():
            for each cell:
                compute pressure at cell using neighbouring a_x values and pressure guesses
        pressure_to_guess():
            for each cell:
                set pressure_guess to pressure


pressure_update():
    for each cell:
        update cell velocity based on surrounding pressure


grid_to_particle():
    for each particle:
        find particle grid pos
        interpolate velocity from the 8 surrounding grid cells around the particle pos
        interpolate old velocity from the 8 surrounding grid cells around the particle pos
        calculate deltaVelocity of the grid and calculate a flip velocity using the particle's velocity
        change the particle velocity, blending between deltaVelocity and velocity of the grid
        

particle_advect():
    for each particle:
        update position of particle using velocity
        add random position to it to prevent squishing
        add external velocity to it
        
