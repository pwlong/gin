`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// file: 			pw_det.v
// author			Paul Long <pwl@pdx.edu> 
// 
// Create Date: 	01/15/2015 09:13:25 PM 
// Module Name: 	pw_det
// Project Name: 	ECE544 Project 1
// Target Devices: 	nexys4
// Tool Versions: 	vivado & sdk 2014.4
// Description: 	This module implements a hardware PWM counter. It returns two 32bit
//					signals corresponding the number of clock ticks the signal was high 
//					and low
// 
// 
// Revision:
// Revision 0.01 - File Created
// 
//////////////////////////////////////////////////////////////////////////////////


module pw_det(
    input               clk,
    input               rst_n,      // expects active low reset
    input               pwm_sig,
    output  reg [31:0]  high_total,
    output  reg [31:0]  low_total);
    
    
    // state definitions
    localparam	RESET_S			= 3'd0,
				WAIT_S			= 3'd1,
				HIGH_S			= 3'd2,
				LOW_S 			= 3'd3,
				END_PERIOD_S	= 3'd4;
    
    // local storage
    reg     [2:0]   present_state;
    reg     [2:0]   next_state;
    reg     [31:0]  high_count;
    reg     [31:0]  low_count;

    
    
    wire rst = ~rst_n;
    
    // sequential state change logic
    always @(posedge clk) begin
        present_state <= rst ? RESET_S : next_state;
    end // always  
/*
    always begin
        high_total = 32'h00000FFF;
        low_total  = 32'h00000FFF;
    end
*/
    // next state and output logic
    always @(posedge clk) begin
        case (present_state)
			RESET_S: begin
				high_count <= 32'd0;
				low_count  <= 32'd0;
				high_total <= 32'd0;
                low_total  <= 32'd0;
				next_state <= WAIT_S;
			end
			
            WAIT_S: begin
                // these reset fudge factors account for some of the high
                // and low counts being consumed by state transitions
                high_count <= 32'd6;
                low_count  <= 32'd2;
                next_state <= pwm_sig ? HIGH_S : WAIT_S;
            end
        
            HIGH_S: begin
                high_count <= pwm_sig ? high_count + 1'd1 : high_count;
                next_state <= ~pwm_sig ? LOW_S : HIGH_S;
            end
            
            LOW_S: begin;
                low_count  <= ~pwm_sig ? low_count + 1'd1 : low_count;
				next_state <= pwm_sig ? END_PERIOD_S : LOW_S;
            end
            
            END_PERIOD_S: begin
				high_total <= high_count;
                low_total  <= low_count;
                next_state <= WAIT_S;
            end
            
            default: begin
                next_state <= RESET_S;
            end
        endcase
    end // always
    

endmodule
