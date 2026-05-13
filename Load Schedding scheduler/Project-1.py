import pandas as pd # Used to analyze large dataset

csv_file = "LS Schedule.csv"
df = pd.read_csv(csv_file)  

def recursive_binary_search_grid(grid_name, grids, left, right):
    # Recursive binary search for a grid in sorted_grids by its name.

    if left > right:
        return []
    
    mid = (left + right) // 2
    if grids[mid][0] == grid_name:
        return grids[mid][1]
    elif grids[mid][0] < grid_name:
        return recursive_binary_search_grid(grid_name, grids, mid + 1, right)
    else:
        return recursive_binary_search_grid(grid_name, grids, left, mid - 1)

    
def fill_grids(dataframe):
    # Returns a tuple to sorted_grids.
    grid_data_dict = {}

    # Iterate over the rows of the dataframe to put together feeders under a same grid name.
    for _, row in dataframe.iterrows():
        # Extract the grid and feeder name from the current row.
        grid_name = row['Grid']  
        feeder_name = row['Feeder Name']
        if grid_name not in grid_data_dict:
            grid_data_dict[grid_name] = []
        grid_data_dict[grid_name].append(feeder_name)
    
    # Convert the dictionary into a list of tuples for sorting.
    grid_data_list = list(grid_data_dict.items())
    # Sorts the grid names for binary search.
    for i in range(len(grid_data_list)):
        for j in range(i + 1, len(grid_data_list)):
            if grid_data_list[i][0] > grid_data_list[j][0]:
                grid_data_list[i], grid_data_list[j] = grid_data_list[j], grid_data_list[i]
    return grid_data_list

def match_time_cycle(time_cycle, feeders):
    # Lookup all feeder names where a specific time cycle contains the input time (partial match).
    matching_feeders = []
    for _, feeder_data in feeders:
        # _ would ignore the feeder name in tuple and grab feeder_data in the dictionary.
        for time in feeder_data["Cycles"].values(): 
            if time_cycle in time:  # Check if the input time is part of the cycle time.
                matching_feeders.append(feeder_data["Feeder Name"])
                break # To avoid adding the same feeder name more than once.
    return matching_feeders

def fill_feeders(dataframe):
    # Returns a tuple to sorted_feeders.
    feeder_data = []
    
    for _, row in dataframe.iterrows(): # iterrows() is a pandas function.
        feeder_name = row['Feeder Name'] 
        # Stores the value from under the 'Feeder Name' column for each row.
        times = {
            "Cycle 1": row.get("1st Cycle", ""),
            "Cycle 2": row.get("2nd Cycle", ""),
            "Cycle 3": row.get("3rd Cycle", ""),
            "Cycle 4": row.get("4th Cycle", "")
        }
        # Gets value under each column for each row.
        feeder_data.append((feeder_name, {"Feeder Name": feeder_name, "Cycles": times})) # list of tuple
    return feeder_data
     # Feeder name and its corresponding data appended as a tuple to the list.
    
def binary_search_feeder(feeder_name, feeders, left, right): 
    # Recursive binary search for a feeder in sorted_feeder by its name.

    if left > right:
        return "Feeder not found."
    
    mid = (left + right) // 2
    if feeders[mid][0] == feeder_name: 
        return feeders[mid][1]
    elif feeders[mid][0] < feeder_name:
        return binary_search_feeder(feeder_name, feeders, mid + 1, right)
    else:
        return binary_search_feeder(feeder_name, feeders, left, mid - 1)

sorted_feeders = fill_feeders(df) 
sorted_grids = fill_grids(df)

# User Input
while True:
    print("Choose an option:")
    print("0. List of all the Grids")
    print("1. Grid to Feeder (Enter a grid name to see all feeders under that grid)")
    print("2. Time to Feeder (Enter a time cycle to find all feeders that have that time)")
    print("3. Feeder to Time (Enter a feeder name to see its time cycles)")
    print("4. Exit!")


    value = input('Enter your choice (0, 1, 2, 3 or 4): ')

    if value=='0':

        print(sorted(set(df['Grid']))) # All unique grid names

    elif value == '1':

        grid = input("Enter the grid name: ")
        feeders = recursive_binary_search_grid(grid, sorted_grids, 0, len(sorted_grids) - 1)
    # sorted_grids is a list of tuples, created by the fill_grids function to sort grid-feeder data.
        if len(feeders) > 0:
            print("Feeders under the grid '" + grid + "':")
            for feeder in feeders:
                print(feeder)

        else:
            print("No feeders found for the grid '" + grid + "'.")

    elif value == '2':

        time_cycle = input('Enter a time cycle (e.g., "0835" or "0835~1105"): ')
        feeders = match_time_cycle(time_cycle, sorted_feeders) 
        # sorted_feeders is a list of tuples, created by the fill_feeders function to sort feeder-time data.
        if feeders:
            print("The feeders with load shedding at", time_cycle, "are:")
            for feeder in feeders:
                print(feeder)
        else:
            print("No feeders found with load shedding at", time_cycle + ".")

    elif value == '3':

        feeder = input('Enter your feeder name (e.g., Feeder 1): ')
        feeder_data = binary_search_feeder(feeder, sorted_feeders, 0, len(sorted_feeders) - 1) 
        # sorted_feeders is a list of tuples, created by the fill_feeders function to sort feeder-time data.
        if feeder_data != "Feeder not found.":
            print("The time cycles for feeder", feeder, "are:", feeder_data["Cycles"])
        else:
            print(feeder_data)

    elif value=='4':

        print('Exit the program. Thank you!')
        break

    else:

        print("Invalid input! Please choose 0, 1, 2, 3 or 4.")