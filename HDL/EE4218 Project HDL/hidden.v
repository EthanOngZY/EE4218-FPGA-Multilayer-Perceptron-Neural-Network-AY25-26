`timescale 1ns / 1ps

module hidden_layer
	#(
		parameter width = 8,
		parameter A_depth_bits = 9,
		parameter hid_depth_bits = 3,
		parameter inter_depth_bits = 7,
		parameter A_rows = 64,
		parameter A_cols = 7,
		parameter hid_rows = 8,
		parameter hid_cols = 2
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
	reg [17:0] sum = 0;
	reg [$clog2(2**A_depth_bits):0] 	read_A_counter 			= 0;
	reg [$clog2(2**hid_depth_bits):0] 	read_hid1_counter 		= 0;
	reg [$clog2(2**hid_depth_bits):0] 	read_hid2_counter 		= 0;
	reg [$clog2(2**inter_depth_bits):0] write_inter_counter 	= 0;
	reg [$clog2(2**inter_depth_bits):0] write_inter_counter2 	= 1;
	
	// Variables
	integer input_bias = 255;
	
	// States
	localparam Hid_Idle 	= 10'b1000000000;
	localparam Hid_Read1 	= 10'b0100000000;
	localparam Hid_Wait1 	= 10'b0010000000;
	localparam Hid_Mult1	= 10'b0001000000;
	localparam Hid_Write1 	= 10'b0000100000;
	localparam Hid_Read2 	= 10'b0000010000;
	localparam Hid_Wait2 	= 10'b0000001000;
	localparam Hid_Mult2	= 10'b0000000100;
	localparam Hid_Write2 	= 10'b0000000010;
	localparam Hid_Done 	= 10'b0000000001;
	reg[9:0] State;
	
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
				sum 					<= 0;
				
				A_read_en 				<= 0;
				hid1_read_en 			<= 0;
				hid2_read_en 			<= 0;
				inter_write_en 			<= 0;
				
				if(Hidden_Start)
				begin
					State 				<= Hid_Read1;
				end
			end
			
			Hid_Read1:
			begin
				A_read_en 				<= 1'b1;
				hid1_read_en 			<= 1'b1;
				A_read_address 			<= read_A_counter;
				hid1_read_address 		<= read_hid1_counter;
				State 					<= Hid_Wait1;
			end
			
			Hid_Wait1:
			begin
				State 					<= Hid_Mult1;
			end
			
			Hid_Mult1:
			begin
				sum 					<= sum + (A_read_data_out * hid1_read_data_out);
				
				if (read_hid1_counter == A_cols)
				begin
					sum 				<= sum + (input_bias*bias_A);
					State 				<= Hid_Write1;
				end
				else
				begin
					read_A_counter 		<= read_A_counter + 1;
					read_hid1_counter 	<= read_hid1_counter + 1;
					State 				<= Hid_Read1;
				end
			end
			
			Hid_Write1:
			begin
				inter_write_en 			<= 1'b1;
				inter_write_address 	<= write_inter_counter;
				
				inter_write_data_in 	<= (sum >> 8);
				
				if (write_inter_counter == A_rows*hid_cols-2)
				begin
					sum 				<= 0;
					hid1_read_en 		<= 0;
					read_A_counter 		<= 0;
					State 				<= Hid_Read2;
				end
				else
				begin
					write_inter_counter <= write_inter_counter + 2;
					read_hid1_counter 	<= 0;
					sum 				<= 0;
					State 				<= Hid_Read1;
				end
			end
			
			Hid_Read2:
			begin
				A_read_en 				<= 1'b1;
				hid2_read_en 			<= 1'b1;
				A_read_address 			<= read_A_counter;
				hid2_read_address 		<= read_hid2_counter;
				State 					<= Hid_Wait2;
			end
			
			Hid_Wait2:
			begin
				State 					<= Hid_Mult2;
			end
			
			Hid_Mult2:
			begin
				sum 					<= sum + (A_read_data_out * hid2_read_data_out);
				
				if (read_hid2_counter == A_cols)
				begin
					sum 				<= sum + (input_bias*bias_B);
					State 				<= Hid_Write2;
				end
				else
				begin
					read_A_counter 		<= read_A_counter + 1;
					read_hid2_counter 	<= read_hid2_counter + 1;
					State 				<= Hid_Read2;
				end
			end
			
			Hid_Write2:
			begin
				inter_write_en 			<= 1'b1;
				inter_write_address 	<= write_inter_counter2;
				
				inter_write_data_in 	<= (sum >> 8);
				
				if (write_inter_counter2 == A_rows*hid_cols-1)
				begin
					State 				<= Hid_Done;
				end
				else
				begin
					write_inter_counter2 <= write_inter_counter2 + 2;
					read_hid2_counter 	<= 0;
					sum 				<= 0;
					State 				<= Hid_Read2;
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