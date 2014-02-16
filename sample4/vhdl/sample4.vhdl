--
-- Copyright (C) 2009-2012 Chris McClelland
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.
--
library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity sample4 is
	port(
		clk_in       : in  std_logic;
		reset_in     : in  std_logic;

		-- DVR interface -----------------------------------------------------------------------------
		chanAddr_in  : in  std_logic_vector(6 downto 0);  -- the selected channel (0-127)

		-- Host >> FPGA pipe:
		h2fData_in   : in  std_logic_vector(7 downto 0);  -- data lines used when the host writes to a channel
		h2fValid_in  : in  std_logic;                     -- '1' means "on the next clock rising edge, please accept the data on h2fData"
		h2fReady_out : out std_logic;                     -- channel logic can drive this low to say "I'm not ready for more data yet"

		-- Host << FPGA pipe:
		f2hData_out  : out std_logic_vector(7 downto 0);  -- data lines used when the host reads from a channel
		f2hValid_out : out std_logic;                     -- channel logic can drive this low to say "I don't have data ready for you"
		f2hReady_in  : in  std_logic;                     -- '1' means "on the next clock rising edge, put your next byte of data on f2hData"

		-- Peripheral interface ----------------------------------------------------------------------
		sample_in    : in    std_logic_vector(3 downto 0) -- sample a nibble on each clock
	);
end entity;

architecture rtl of sample4 is
	-- Read FIFO:
	signal fifoInputData   : std_logic_vector(7 downto 0);  -- producer: data
	signal fifoInputValid  : std_logic;                     --           valid flag
	signal fifoInputReady  : std_logic;                     --           ready flag
	signal fifoOutputData  : std_logic_vector(7 downto 0);  -- consumer: data
	signal fifoOutputValid : std_logic;                     --           valid flag
	signal fifoOutputReady : std_logic;                     --           ready flag

	signal sync1, sync2    : std_logic_vector(3 downto 0) := (others => '0');
	signal toggle          : std_logic := '0';
begin
	-- Infer registers
	process(clk_in)
	begin
		if ( rising_edge(clk_in) ) then
			if ( reset_in = '1' ) then
				sync1 <= (others => '0');
				sync2 <= (others => '0');
				toggle <= '0';
			else
				sync1 <= sample_in;
				sync2 <= sync1;
				toggle <= not toggle;
			end if;
		end if;
	end process;

	-- Wire up read FIFO to channel 0 reads:
	--   fifoInputValid driven by producer_timer
	--   flags(0) driven by fifoInputReady
	fifoInputData <= sync2(0) & sync2(1) & sync2(2) & sync2(3) & sync1(0) & sync1(1) & sync1(2) & sync1(3);
	fifoInputValid <= toggle;

	f2hValid_out <=
		'0' when fifoOutputValid = '0' and chanAddr_in = "0000000"
		else '1';
	fifoOutputReady <=
		'1' when f2hReady_in = '1' and chanAddr_in = "0000000"
		else '0';
	
	-- Select values to return for each channel when the host is reading
	with chanAddr_in select f2hData_out <=
		fifoOutputData when "0000000",  -- get from read FIFO
		x"00"          when others;
	
	-- Read FIFO: written by counter, read by host
	read_fifo : entity work.fifo_wrapper
		port map(
			clk_in          => clk_in,

			-- Production end
			inputData_in    => fifoInputData,
			inputValid_in   => fifoInputValid,
			inputReady_out  => fifoInputReady,

			-- Consumption end
			outputData_out  => fifoOutputData,
			outputValid_out => fifoOutputValid,
			outputReady_in  => fifoOutputReady
		);

end architecture;
