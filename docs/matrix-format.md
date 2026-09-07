# Appendix: Matrix-element format

Matrix elements are expected in the following forms:

```text
[+/-][real_part][+/-][i][img_part]          Example: +2.3-i0.2
[+/-][real_part][+/-][img_part][i]          Example: -2.3-0.2i
[real_part][+/-][i][img_part]               Example:  2.3-i0.2
[real_part][+/-][img_part][i]               Example:  2.3-0.2i
[+/-][i][img_part][+/-][real_part]          Example: +i2.3-0.2
[+/-][img_part][i][+/-][real_part]          Example: -2.3i-0.2
[i][img_part][+/-][real_part]               Example: i2.3-0.2
[img_part][i][+/-][real_part]               Example: +2.3i+0.2
[+/-][real_part]                            Example: -2.3
[real_part]                                 Example:  2.3
[+/-][img_part][i]                          Example: +2.3i
[+/-][i][img_part]                          Example: -i2.3
[img_part][i]                               Example:  2.3i
[i][img_part]                               Example: i2.3
```

- Note: `i` without a number is interpreted as `1*i`, e.g. `3+i` = `3+1i`.

Elements in a row are separated by one or more whitespace characters; a newline starts a new row.