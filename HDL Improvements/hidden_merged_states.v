`timescale 1ns / 1ps

module hidden_layer
	#(
		parameter width 			= 8,
		parameter A_depth_bits 		= 9,
		parameter hid_depth_bits 	= 3,
		parameter inter_depth_bits 	= 7,
		parameter A_rows 			= 64,
		parameter A_cols 			= 7,
		parameter hid_rows 			= 8,
		parameter hid_cols 			= 2
	)
	(
		input 			clk,
		input 			Hidden_Start,
		output 	reg 	Hidden_Done,
	
		output 	reg 	A_read_en,
		output 	reg 	[A_depth_bits-1:0] A_read_address,
		input	wire 	[width-1:0] A_read_data_out,
	
		output 	reg 	hid1_read_en,
		output 	reg 	[hid_depth_bits-1:0] hid1_read_address,
		input 	wire 	[width-1:0] hid1_read_data_out,
		
		output 	reg 	hid2_read_en,
		output 	reg 	[hid_depth_bits-1:0] hid2_read_address,
		input 	wire 	[width-1:0] hid2_read_data_out,
		
		output 	reg 	inter_write_en,
		output 	reg 	[inter_depth_bits-1:0] inter_write_address,
		output 	reg 	[width-1:0] inter_write_data_in,
	
		input [7:0] 	bias_A,
		input [7:0] 	bias_B
	);
	
	// Sum and Counters
	reg [17:0] 							sum1 					= 0;
	reg [17:0] 							sum2 					= 0;
	reg [$clog2(2**A_depth_bits):0] 	read_A_counter 			= 0;
	reg [$clog2(2**hid_depth_bits):0] 	read_hid1_counter 		= 0;
	reg [$clog2(2**hid_depth_bits):0] 	read_hid2_counter 		= 0;
	reg [$clog2(2**inter_depth_bits):0] write_inter_counter 	= 0;
	
	// Variables
	integer input_bias = 255;
	
	// States
	localparam Hid_Idle 	= 7'b1000000;
	localparam Hid_Read 	= 7'b0100000;
	localparam Hid_Wait 	= 7'b0010000;
	localparam Hid_Mult		= 7'b0001000;
	localparam Hid_Write1 	= 7'b0000100;
	localparam Hid_Write2 	= 7'b0000010;
	localparam Hid_Done 	= 7'b0000001;
	reg[6:0] State;
	
	always @(posedge clk)
	begin
		case(State)
			Hid_Idle:
			begin
				Hidden_Done 			<= 0;
				read_A_counter 			<= 0;
				read_hid1_counter 		<= 0;
				read_hid2_counter 		<= 0;
				
				A_read_address 			<= 0;
				hid1_read_address 		<= 0;
				hid2_read_address 		<= 0;
				inter_write_address 	<= 0;
				inter_write_data_in 	<= 0;
				sum1 					<= 0;
				sum2					<= 0;
				
				A_read_en 				<= 0;
				hid1_read_en 			<= 0;
				hid2_read_en 			<= 0;
				inter_write_en 			<= 0;
				
				if(Hidden_Start)
				begin
					State 				<= Hid_Read;
				end
			end
			
			Hid_Read:
			begin
				A_read_en 				<= 1'b1;
				hid1_read_en 			<= 1'b1;
				hid2_read_en			<= 1'b1;
				A_read_address 			<= read_A_counter;
				hid1_read_address 		<= read_hid1_counter;
				hid2_read_address		<= read_hid2_counter;
				State 					<= Hid_Wait;
			end
			
			Hid_Wait:
			begin
				State 					<= Hid_Mult;
			end
			
			Hid_Mult:
			begin
				sum1 					<= sum1 + (A_read_data_out * hid1_read_data_out);
				sum2					<= sum2 + (A_read_data_out * hid2_read_data_out);
				
				if (read_hid1_counter == A_cols)
				begin
					sum1 				<= sum1 + (input_bias*bias_A);
					sum2				<= sum2 + (input_bias*bias_B);
					State 				<= Hid_Write1;
				end
				else
				begin
					read_A_counter 		<= read_A_counter + 1;
					read_hid1_counter 	<= read_hid1_counter + 1;
					read_hid2_counter	<= read_hid2_counter + 1;
					State 				<= Hid_Read;
				end
			end
			
			Hid_Write1:
			begin
				inter_write_en 			<= 1'b1;
				inter_write_address 	<= write_inter_counter;
				inter_write_data_in 	<= (sum1 >> 8);
				
				write_inter_counter		<= write_inter_counter + 1;
				State					<= Hid_Write2;
			end
			
			Hid_Write2:
			begin
				inter_write_en 			<= 1'b1;
				inter_write_address 	<= write_inter_counter;				
				inter_write_data_in 	<= (sum2 >> 8);
				
				if (write_inter_counter == A_rows*hid_cols - 1)
				begin
					State 				<= Hid_Done;
				end
				else
				begin
					write_inter_counter <= write_inter_counter + 1;
					read_hid1_counter	<= 0;
					read_hid2_counter 	<= 0;
					sum1 				<= 0;
					sum2				<= 0;
					State 				<= Hid_Read;
				end
			end
			
			Hid_Done:
			begin
				inter_write_en 			<= 0;
				A_read_en 				<= 0;
				hid2_read_en 			<= 0;
				Hidden_Done 			<= 1'b1;
				
				if (!Hidden_Start)
					State 				<= Hid_Idle;
			end
			
			default: State 				<= Hid_Idle;
		endcase
	end
endmodule