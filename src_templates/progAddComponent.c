#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fileExists(const char *path)
{
    FILE *file = fopen(path, "r");
    if (file)
    {
        fclose(file);
        return 1;
    }
    return 0;
}

void writeTemplateContent(FILE *tplFile, FILE *targetFile, const char *componentName)
{
    ssize_t read;
    int componentNameLen = strlen(componentName);
    int reading = 1;
    while (reading)
    {
        char line[1024];
        int readPos = 0;
        int c;
        while (c = fgetc(tplFile))
        {
            if (c == EOF)
            {
                reading = 0;
                break;
            }
            if (readPos >= sizeof(line) / 2)
            {
                printf("template line too long\n");
                return;
            }
            line[readPos++] = c;
            if (c == '\n')
            {
                break;
            }
        }
        line[readPos] = '\0';
        char newLine[1024];
        int i = 0;
        int j = 0;
        while (line[i] != '\0')
        {
            if (line[i] == '$' && line[i + 1] == '{' && strncmp(line + i + 2, "COMPONENT_NAME}", 15) == 0)
            {
                i += 3 + 14;
                for (int k = 0; k < componentNameLen; k++)
                {
                    newLine[j++] = componentName[k];
                }
            }
            else
            {
                newLine[j++] = line[i++];
            }
        }

        newLine[j] = '\0';
        fprintf(targetFile, "%s", newLine);
    }
}

int main(int argc, char const *argv[])
{
    const char *targetDir = NULL;
    const char *componentName = NULL;
    if (argc < 3)
    {
        printf("Usage: progAddComponent <targetDir> <componentName>\n");
        return 1;
    }

    int force = 0;
    if (argc > 3 && strcmp(argv[3], "-f") == 0)
    {
        force = 1;
    }

    FILE *tplSource = fopen("src_templates/component.tpl_c", "rb");
    FILE *tplHeader = fopen("src_templates/component.tpl_h", "rb");
    if (!tplSource || !tplHeader)
    {
        printf("Failed to open template files\n");
        return 1;
    }

    targetDir = argv[1];
    componentName = argv[2];
    char *headerPath = (char *)malloc(strlen(targetDir) + strlen(componentName) + 32);
    char *sourcePath = (char *)malloc(strlen(targetDir) + strlen(componentName) + 32);
    sprintf(headerPath, "%s/%s.h", targetDir, componentName);
    sprintf(sourcePath, "%s/%s.c", targetDir, componentName);
    
    // check if file exists
    if (!force && fileExists(headerPath))
    {
        printf("Component header file already exists: %s\n", headerPath);
        return 1;
    }
    if (!force && fileExists(sourcePath))
    {
        printf("Component source file already exists: %s\n", sourcePath);
        return 1;
    }
    FILE *headerFile = fopen(headerPath, "wb");
    FILE *sourceFile = fopen(sourcePath, "wb");
    if (!headerFile || !sourceFile)
    {
        printf("Failed to create files\n");
        return 1;
    }

    printf("Creating component %s in %s\n", componentName, targetDir);
    printf("  Header file: %s\n", headerPath);
    writeTemplateContent(tplHeader, headerFile, componentName);

    printf("  Source file: %s\n", sourcePath);
    writeTemplateContent(tplSource, sourceFile, componentName);

    fclose(tplHeader);
    fclose(tplSource);
    fclose(headerFile);
    fclose(sourceFile);
    free(headerPath);
    free(sourcePath);
    
    return 0;
}
