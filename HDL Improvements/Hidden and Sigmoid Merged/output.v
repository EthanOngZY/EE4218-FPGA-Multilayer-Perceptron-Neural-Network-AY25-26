`timescale 1ns / 1ps

module output_layer
#(
    parameter width 				= 8,
    parameter out_depth_bits 		= 2,
    parameter RES_depth_bits 		= 6,
    parameter inter_depth_bits 		= 7,
    parameter A_rows 				= 64,
    parameter hid_cols 				= 2,
    parameter out_rows 				= 3,
    parameter out_cols 				= 1
)
(
    input 			clk,
    input 			Start_Output,
    output 	reg 	Done_Output,

	output 	reg 	inter_read_en,
	output 	reg 	[inter_depth_bits-1:0] inter_read_address,
	input	wire 	[width-1:0] inter_read_data_out,
	
	output 	reg 	out_read_en,
	output 	reg 	[out_depth_bits-1:0] out_read_address,
	input 	wire 	[width-1:0] out_read_data_out,
	
	output 	reg 	RES_write_en,
	output 	reg 	[RES_depth_bits-1:0] RES_write_address,
	output 	reg 	[width-1:0] RES_write_data_out,
    
	input 	[7:0] 	bias_C
);
	
	// Sums and Counters
	reg [17:0] 								sum 					= 0;
	reg [$clog2(2**inter_depth_bits):0] 	read_inter_counter 	= 0;
	reg [$clog2(2**out_depth_bits):0] 		read_out_counter 		= 0;
	reg [$clog2(2**RES_depth_bits):0] 		write_RES_counter 		= 0;
	
	// Variables
	integer input_bias 		= 255;
	
	// States
	localparam Out_Idle 	= 6'b100000;
	localparam Out_Read		= 6'b010000;
	localparam Out_Wait 	= 6'b001000;
	localparam Out_Mult		= 6'b000100;
	localparam Out_Write 	= 6'b000010;
	localparam Out_Done 	= 6'b000001;
	reg[5:0] State;
	
	always @(posedge clk)
	begin
		case(State)
			Out_Idle:
			begin
				Done_Output 			<= 0;
				read_inter_counter 		<= 0;
				read_out_counter 		<= 0;
				write_RES_counter 		<= 0;
				
				inter_read_address 		<= 0;
				out_read_address 		<= 0;
				RES_write_address 		<= 0;
				sum 					<= 0;
				
				inter_read_en 			<= 0;
				out_read_en 			<= 0;
				RES_write_en 			<= 0;
				
				if(Start_Output)
				begin
					State 				<= Out_Read;
				end
			end
			
			Out_Read:
			begin
				inter_read_en 			<= 1'b1;
				out_read_en 			<= 1'b1;
				inter_read_address 		<= read_inter_counter;
				out_read_address 		<= read_out_counter;
				State 					<= Out_Wait;
			end
			
			Out_Wait:
			begin
				State 					<= Out_Mult;
			end
			
			Out_Mult:
			begin
				sum 					<= sum + (inter_read_data_out * out_read_data_out);
				
				if (read_out_counter == out_rows-1)
				begin
					sum 				<= sum + (input_bias*bias_C);
					State 				<= Out_Write;
				end
				else 
				begin
					read_out_counter 	<= read_out_counter + 1;
					read_inter_counter 	<= read_inter_counter + 1;
					State 				<= Out_Read;
				end
			end
			
			Out_Write:
			begin
				if ((sum/256) < 128)
					RES_write_data_out 	<= 00;
				else
				begin
					RES_write_data_out 	<= 01;
				end
				
				RES_write_en 			<= 1;
				RES_write_address 		<= write_RES_counter;
				
				if (write_RES_counter == A_rows-1)
				begin
					State 				<= Out_Done;
				end
				else
				begin
					write_RES_counter 	<= write_RES_counter + 1;
					//read_inter2_counter <= read_inter2_counter;
					read_out_counter 	<= 0;
					sum 				<= 0;
					State 				<= Out_Read;
				end
			end
			
			Out_Done:
			begin
				RES_write_en 			<= 0;
				Done_Output 			<= 1;
				
				if (!Start_Output)
					State 				<= Out_Idle;
			end
			
			default: State 				<= Out_Idle;
		endcase
	end
endmodule