Usage
-----

-   Compile

        make

-   Execute

        ./r00922006_vlc_dec.exe huff_table_name compressed.txt decoded_symbol.txt

File Format
-----------

-   Huffman table

    每個 symbol-codeword pair 為兩行，第一行為 symbol ，第二行為 codeword ， symbol 和 codeword 都以 character 的方式儲存。

-   Compressed text file

    以 character 儲存的 bit stream ，如同助教給的 `compressed.txt` ，為了測式方便，在讀入 bit stream 時，如果遇到換行會自動忽略，因此不論 input file 是一個 symbol codeword 一行或是所有 bit stream 在同一行都能順利 decode。

-   Decoded text file

    一個 symbol 一行，以 character 型式儲存於 file 中，如同助教給的 `decoded_symbol.txt` 。

Modification
------------

在 [Memory Efficient and High-Speed Search Huffman Coding][1] 裡提到， LUT (look up table) 裡的 entry 個數是變動的，為了方便實作，我把每個 LUT 裡都放了 16 個 entry ，這樣雖然會在 LUT 多花一些空間，但對於 SGH-Tree(single-side growing Huffman Tree) 來說，entry 個數 < 16 個的 LUT 出現機率並不高 (<0.5) ，故這樣子的實作我認為是可以接受的。

Improvement
-----------

在 [Memory Efficient and High-Speed Search Huffman Coding][1] 裡，有個 Super-Table 的 data structure ，在我的實作是直接在 LUT 裡的，這點也是我覺得 paper 裡很奇怪的一點，為什麼不能把需要的資料直接寫在 LUT 裡，而需要去另外查表？

另外 paper 用了一個 sparse 的 symbol table ，在我的實作裡，用 std::vector 使用了一個 condense symbol table ，在 LUT 裡則是存了 symbol table element 的 index 。

照理來說要做壓縮，應該是要壓成 0101... 的 bit stream ，不過因為作業的 sample input (compressed.txt) 是將 bit stream 存成 character ，因為在我的實作裡也是把 codeword 及 symbol 都存成 character 的型式。

Other Implement
---------------

在實作 [Memory Efficient and High-Speed Search Huffman Coding][1] 時，為了測式方便，我也順便把 paper 中的範例 Huffman Table 建好，存為 `K7` ，也寫一個範例的 `compressed_K7.txt` ，裡面是所有 `K7` symbol 壓縮後的 bit stream 。



[1]:http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=469442&tag=1
