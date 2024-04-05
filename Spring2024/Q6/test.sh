# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test.sh                                            :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vvaucoul <vvaucoul@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/03/27 11:12:36 by vvaucoul          #+#    #+#              #
#    Updated: 2024/03/27 11:25:07 by vvaucoul         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

gcc main.c -o test -ljson-c
printf "Test 1\n"
time ./test < tests/Test_1_input.txt
printf "\nTest 2\n"
time ./test < tests/Test_2_input.txt
printf "\nTest 3\n"
time ./test < tests/Test_3_input.txt
printf "\nTest 4\n"
time ./test < tests/Test_4_input.txt
printf "\nTest 5\n"
time ./test < tests/Test_5_input.txt
printf "\nTest 6\n"
time ./test < tests/Test_6_input.txt
printf "\nTest 7\n"
time ./test < tests/Test_7_input.txt
printf "\nTest 8\n"
time ./test < tests/Test_8_input.txt