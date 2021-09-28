module top(b0, b1, a0, a1, s0, s1, c1);
input b0, b1, a0, a1;
output s0, s1, c1;

wire w8, w9, w10, w11, w12, w13, w14, w15, w16, w17, w18, w19, w20, w21, w22, w23, w24;
wire w8_c, w9_c, w10_c, w11_c, w12_c, w13_c, w14_c, w15_c, w16_c, w17_c, w18_c, w19_c, w20_c, w21_c, w22_c, w23_c, w24_c;
wire b0_c, b1_c, a0_c, a1_c;
assign b0_c = ~b0;
assign b1_c = ~b1;
assign a0_c = ~a0;
assign a1_c = ~a1;

assign w8_c = ~w8;
assign w9_c = ~w9;
assign w10_c = ~w10;
assign w11_c = ~w11;
assign w12_c = ~w12;
assign w13_c = ~w13;
assign w14_c = ~w14;
assign w15_c = ~w15;
assign w16_c = ~w16;
assign w17_c = ~w17;
assign w18_c = ~w18;
assign w19_c = ~w19;
assign w20_c = ~w20;
assign w21_c = ~w21;
assign w22_c = ~w22;
assign w23_c = ~w23;
assign w24_c = ~w24;

and (w8, b0, a0_c);
and (w9, b0_c, a0);
and (w10, w8_c, w9_c);
and (w11, b0, a0);
and (w12, w11, b1);
and (w13, b1_c, w11_c);
and (w14, w13_c, w12_c);
and (w15, w14_c, a1);
and (w16, w11, b1_c);
and (w17, w11_c, b1);
and (w18, w16_c, w17_c);
and (w19, w18_c, a1_c);
and (w20, w15_c, w19_c);
and (w21, w16_c, b1_c);
and (w22, w21_c, a1);
and (w23, w12, a1_c);
and (w24, w23_c, w22_c);
buf (s0, w10_c);
buf (s1, w20_c);
buf (c1, w24_c);

endmodule



