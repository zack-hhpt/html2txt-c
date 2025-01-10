#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Function declarations
void html_to_text(const char *html, char *text);
void decode_html_entities(const char *src, char *dest);
bool is_valid_tag_char(char c);
bool is_valid_tag_start(const char *tag);
bool is_html_file(const char *filename);
void print_help(const char *prog_name);

// Check if the file is an HTML file
bool is_html_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    return ext && (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0);
}

// Print help message
void print_help(const char *prog_name) {
    printf("Usage: %s [<input_file>] [<output_file>]\n", prog_name);
    printf("If no input_file is provided, stdin is used.\n");
    printf("If no output_file is provided, stdout is used.\n");
    printf("Options:\n");
    printf("  -h    Show this help message\n");
}

// Main function
int main(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *output_file = NULL;
    FILE *infile = stdin;
    FILE *outfile = stdout;

    if (argc > 3) {
        print_help(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            print_help(argv[0]);
            return 0;
        }
    }

    if (argc >= 2) {
        input_file = argv[1];
    }

    if (argc == 3) {
        output_file = argv[2];
    }

    if (input_file) {
        infile = fopen(input_file, "r");
        if (infile == NULL) {
            perror("Error opening input file");
            return 1;
        }
    }

    fseek(infile, 0, SEEK_END);
    long length = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    char *content = malloc(length + 1);
    if (content == NULL) {
        perror("Error allocating memory");
        if (infile != stdin) fclose(infile);
        return 1;
    }

    fread(content, 1, length, infile);
    content[length] = '\0';
    if (infile != stdin) fclose(infile);

    char *text = NULL;
    if (input_file && is_html_file(input_file)) {
        text = malloc(length + 1);
        if (text == NULL) {
            perror("Error allocating memory");
            free(content);
            return 1;
        }
        html_to_text(content, text);
        free(content);
    } else {
        text = content;
    }

    if (output_file) {
        outfile = fopen(output_file, "w");
        if (outfile == NULL) {
            perror("Error opening output file");
            free(text);
            return 1;
        }
    }

    fprintf(outfile, "%s", text);
    if (outfile != stdout) fclose(outfile);
    free(text);

    return 0;
}

// Function to convert HTML to plain text
void html_to_text(const char *html, char *text) {
    bool in_tag = false;
    bool in_style = false;
    bool in_script = false;
    bool in_comment = false;
    int j = 0;
    int length = strlen(html);
    char *temp = malloc(length + 1);

    if (temp == NULL) {
        perror("Error allocating memory");
        exit(1);
    }

    for (int i = 0; html[i] != '\0'; i++) {
        if (in_style) {
            if (strncmp(&html[i], "</style>", 8) == 0) {
                in_style = false;
                i += 7; // Skip "</style>"
            }
            continue;
        }

        if (in_script) {
            if (strncmp(&html[i], "</script>", 9) == 0) {
                in_script = false;
                i += 8; // Skip "</script>"
            }
            continue;
        }

        if (in_comment) {
            if (strncmp(&html[i], "-->", 3) == 0) {
                in_comment = false;
                i += 2; // Skip "-->"
            }
            continue;
        }

        if (html[i] == '<') {
            int tag_start = i + 1;
            while (html[tag_start] && isspace(html[tag_start])) {
                tag_start++;
            }

            if (is_valid_tag_start(&html[tag_start])) 
            {
                if (strncmp(&html[i], "<br>", 4) == 0) {
                    temp[j++] = '\n';
                    i += 3; // Skip "br>"
                } else if (strncmp(&html[tag_start], "style", 5) == 0) {
                    in_style = true;
                }
                else if (strncmp(&html[tag_start], "script", 6) == 0) {
                    in_script = true;
                } else if (strncmp(&html[tag_start], "!--", 3) == 0) {
                    in_comment = true;
                }
                else
                {
                    in_tag = true;
                }
            }
            else
            {
                temp[j++] = html[i];
            }
        } 
        else if (!in_tag) {
            temp[j++] = html[i];
        }
        else if (html[i] == '>') 
        {
            in_tag = false;
        }
    }
    temp[j] = '\0';
    decode_html_entities(temp, text);
    free(temp);
}

// Function to decode HTML entities
void decode_html_entities(const char *src, char *dest) {
    int i = 0, j = 0;
    while (src[i]) {
        if (src[i] == '&') {
            if (strncmp(&src[i], "&nbsp;", 6) == 0) {
                dest[j++] = ' ';
                i += 6;
            } else if (strncmp(&src[i], "&lt;", 4) == 0) {
                dest[j++] = '<';
                i += 4;
            } else if (strncmp(&src[i], "&gt;", 4) == 0) {
                dest[j++] = '>';
                i += 4;
            } else if (strncmp(&src[i], "&amp;", 5) == 0) {
                dest[j++] = '&';
                i += 5;
            } else if (strncmp(&src[i], "&quot;", 6) == 0) {
                dest[j++] = '"';
                i += 6;
            } else if (strncmp(&src[i], "&apos;", 6) == 0) {
                dest[j++] = '\'';
                i += 6;
            } else {
                dest[j++] = src[i++];
            }
        } else {
            dest[j++] = src[i++];
        }
    }
    dest[j] = '\0';
}

// Check if a character is a valid tag character
bool is_valid_tag_char(char c) {
    return isalnum(c) || c == '/' || c == '!';
}

// Check if the tag start is valid
bool is_valid_tag_start(const char *tag) {
    // Tag name must start with a letter or exclamation mark, not a number
    if (!isalpha(tag[0]) && tag[0] != '/' && tag[0] != '!') {
        return false;
    }

    // Check if other characters in the tag name are valid
    for (int i = 1; tag[i] && !isspace(tag[i]) && tag[i] != '>'; i++) {
        if (!is_valid_tag_char(tag[i])) {
            return false;
        }
    }

    return true;
}