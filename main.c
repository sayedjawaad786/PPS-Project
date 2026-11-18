//To make the program work opena new terminal and run these commands
//gcc -o forecast main.c -lm
//./forecast
/*
=========================================================
   PROJECT 25: INVENTORY FORECASTING TOOL (ML-LIKE)      
=========================================================

Enter Item ID: 101
Enter Item Name: Wireless Mouse
Enter Lead Time (in days): 3
Enter Safety Stock (units): 20
Enter Current Inventory Level (units): 60
How many days of historical sales data to enter? 5
Enter the daily sales for the last 5 days (separated by space):
10 12 15 14 18

Enter forecast window (n-days, e.g., 3 or 7): 3

--- FORECASTING RESULTS (Next Period Demand) ---
Moving Average (3-day): 15.67 units
Weighted Moving Average (3-day): 16.17 units

--- ERROR METRICS (MAPE) ---
MA MAPE: 17.99%
WMA MAPE: 14.09%

-> Using WMA for recommendations based on lower MAPE.

--- REORDER RECOMMENDATIONS ---
Reorder Level Suggestion: 69 units
Recommended Order Quantity: 103 units
Expected Stock-Out In: 3.7 Days
Reorder Date: ** IMMEDIATE ACTION REQUIRED! **
Current inventory (60) is below reorder level (69).
---------------------------------------------------------

[System] Dynamic memory freed successfully. Exiting.
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Syllabus Topics Applied: Structures[cite: 1]
typedef struct {
    int item_id;
    char item_name[50];
    double *sales;        // Pointer for dynamic time-series storage[cite: 1]
    int num_days;
    int lead_time;
    int safety_stock;
    int current_inventory;
} InventoryItem;

// --- Function Prototypes ---
void input_item_data(InventoryItem *item);
double calc_moving_average(double *sales, int num_days, int window);
double calc_weighted_moving_average(double *sales, int num_days, int window);
double calc_mape(double *sales, int num_days, int window, int is_weighted);
void generate_recommendations(InventoryItem *item, double forecast_demand);

int main() {
    printf("=========================================================\n");
    printf("   PROJECT 25: INVENTORY FORECASTING TOOL (ML-LIKE)      \n");
    printf("=========================================================\n\n");

    InventoryItem item;
    
    // 1. Load Data & Allocate Memory
    input_item_data(&item);

    int window;
    printf("\nEnter forecast window (n-days, e.g., 3 or 7): ");
    scanf("%d", &window);

    if (window > item.num_days || window <= 0) {
        printf("Error: Window must be between 1 and %d\n", item.num_days);
        free(item.sales); // Free allocated memory before exit
        return 1;
    }

    // 2. Forecasting Engine (MA & WMA)[cite: 1]
    double ma_forecast = calc_moving_average(item.sales, item.num_days, window);
    double wma_forecast = calc_weighted_moving_average(item.sales, item.num_days, window);

    // 3. Error Metrics Compare (MAPE)[cite: 1]
    double ma_mape = calc_mape(item.sales, item.num_days, window, 0);
    double wma_mape = calc_mape(item.sales, item.num_days, window, 1);

    printf("\n--- FORECASTING RESULTS (Next Period Demand) ---\n");
    printf("Moving Average (%d-day): %.2f units\n", window, ma_forecast);
    printf("Weighted Moving Average (%d-day): %.2f units\n", window, wma_forecast);
    printf("\n--- ERROR METRICS (MAPE) ---\n");
    printf("MA MAPE: %.2f%%\n", ma_mape);
    printf("WMA MAPE: %.2f%%\n", wma_mape);

    // Select the best model (lowest error) for recommendations
    double best_forecast = (wma_mape < ma_mape) ? wma_forecast : ma_forecast;
    printf("\n-> Using %s for recommendations based on lower MAPE.\n", 
          (wma_mape < ma_mape) ? "WMA" : "MA");

    // 4. Reorder Insights & Recommendations[cite: 1]
    generate_recommendations(&item, best_forecast);

    // Clean up dynamic memory[cite: 1]
    free(item.sales);
    printf("\n[System] Dynamic memory freed successfully. Exiting.\n");

    return 0;
}

// Function to handle user input and dynamic memory allocation
void input_item_data(InventoryItem *item) {
    printf("Enter Item ID: ");
    scanf("%d", &item->item_id);
    
    printf("Enter Item Name: ");
    scanf(" %[^\n]s", item->item_name); 
    
    printf("Enter Lead Time (in days): ");
    scanf("%d", &item->lead_time);
    
    printf("Enter Safety Stock (units): ");
    scanf("%d", &item->safety_stock);
    
    printf("Enter Current Inventory Level (units): ");
    scanf("%d", &item->current_inventory);

    printf("How many days of historical sales data to enter? ");
    scanf("%d", &item->num_days);

    // Dynamic memory allocation for historical sales array[cite: 1]
    item->sales = (double *)malloc(item->num_days * sizeof(double));
    if (item->sales == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    printf("Enter the daily sales for the last %d days (separated by space):\n", item->num_days);
    for (int i = 0; i < item->num_days; i++) {
        scanf("%lf", &item->sales[i]);
    }
}

// Function to calculate Simple Moving Average[cite: 1]
double calc_moving_average(double *sales, int num_days, int window) {
    double sum = 0.0;
    for (int i = num_days - window; i < num_days; i++) {
        sum += sales[i];
    }
    return sum / window;
}

// Function to calculate Weighted Moving Average (more weight to recent days)[cite: 1]
double calc_weighted_moving_average(double *sales, int num_days, int window) {
    double sum = 0.0;
    int weight_sum = (window * (window + 1)) / 2;
    int current_weight = 1;
    
    for (int i = num_days - window; i < num_days; i++) {
        sum += sales[i] * current_weight;
        current_weight++;
    }
    return sum / weight_sum;
}

// Function to calculate Mean Absolute Percentage Error (MAPE)[cite: 1]
double calc_mape(double *sales, int num_days, int window, int is_weighted) {
    if (num_days <= window) return 0.0; // Not enough data to test error

    double total_percentage_error = 0.0;
    int test_cases = 0;

    // Slide through the historical data to test the forecast against actuals
    for (int i = window; i < num_days; i++) {
        double forecast;
        if (is_weighted) {
            forecast = calc_weighted_moving_average(sales, i, window);
        } else {
            forecast = calc_moving_average(sales, i, window);
        }
        
        double actual = sales[i];
        if (actual == 0) actual = 1; // Prevent division by zero
        
        double error = fabs(actual - forecast) / actual;
        total_percentage_error += error;
        test_cases++;
    }

    return (total_percentage_error / test_cases) * 100.0;
}

// Function to calculate and present order quantity, stock-out days, and reorder date[cite: 1]
void generate_recommendations(InventoryItem *item, double forecast_demand) {
    printf("\n--- REORDER RECOMMENDATIONS ---\n");

    if (forecast_demand <= 0) forecast_demand = 1; // Safeguard

    // Reorder Level = (Lead Time * Avg Daily Demand) + Safety Stock[cite: 1]
    double reorder_level = (item->lead_time * forecast_demand) + item->safety_stock;
    
    // Expected Stock-Out Days[cite: 1]
    double expected_stockout_days = item->current_inventory / forecast_demand;
    
    // Reorder Date Calculation[cite: 1]
    // How many days from today until we hit the Reorder Level?
    double days_until_reorder = (item->current_inventory - reorder_level) / forecast_demand;

    // Suggested Order Quantity = (Desired max level/EOQ). Keeping it simple: 
    // Reorder enough to cover lead time + next cycle + safety stock[cite: 1]
    double order_quantity = reorder_level * 1.5; 

    printf("Reorder Level Suggestion: %.0f units\n", ceil(reorder_level));
    printf("Recommended Order Quantity: %.0f units\n", ceil(order_quantity));
    printf("Expected Stock-Out In: %.1f Days\n", expected_stockout_days);

    if (item->current_inventory <= reorder_level) {
        printf("Reorder Date: ** IMMEDIATE ACTION REQUIRED! **\n");
        printf("Current inventory (%d) is below reorder level (%.0f).\n", 
               item->current_inventory, ceil(reorder_level));
    } else {
        printf("Reorder Date: In approximately %.0f days.\n", floor(days_until_reorder));
    }
    printf("---------------------------------------------------------\n");
}