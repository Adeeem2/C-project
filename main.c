#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
// Q-1.3
void hashFile(char* source , char*dest){
    FILE* file = fopen(dest, "w");
    if (file == NULL) {
        perror("Erreur d'ouverture du fichier de destination");
        return;
    }
    static char template[] = "/tmp/myfileXXXXXX";
    char fname[1000];
    strcpy(fname, template);
    int fd = mkstemp(fname);
    if (fd == -1) {
        perror("Erreur de création du fichier temporaire");
        return;
    }

    char command[512];
    snprintf(command, sizeof(command), "sha256sum '%s' | cut -d ' ' -f 1", source); // store hash in command
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        perror("Error opening pipe");
        return;
    }
    char hash[256];
    if (fgets(hash, sizeof(hash), pipe) != NULL) {
        fprintf(file, "%s", hash);
    } else {
        perror("Error reading command output");
    }
    fclose(file);
    printf("Hash written to %s\n", dest);
    close(fd);
}

// Q-1.4
char* sha256file(char* file) {
    static char template[] = "/tmp/myfileXXXXXX";
    char fname[1000];
    strcpy(fname, template);
    int fd = mkstemp(fname);
    if (fd == -1) {
        perror("Erreur de création du fichier temporaire");
        return NULL;
    }
    close(fd);

    char command[512];
    snprintf(command, sizeof(command), "sha256sum '%s' > '%s'", file, fname);
    int ret = system(command);
    if (ret == -1) {
        perror("Erreur lors de l'exécution de la commande");
        unlink(fname);
        return NULL;
    }

    FILE* tempFile = fopen(fname, "r");
    if (tempFile == NULL) {
        perror("Erreur d'ouverture du fichier temporaire");
        unlink(fname);
        return NULL;
    }

    static char hash[256];
    if (fscanf(tempFile, "%255s", hash) != 1) {
        perror("Erreur de lecture du hash");
        fclose(tempFile);
        unlink(fname);
        return NULL;
    }

    fclose(tempFile);
    unlink(fname);
    return hash;
}
// Q-2.1
typedef struct {
    char* data;
    struct cell* next;
}Cell;
typedef Cell* List;

List* initList(){
    List* list = (List*)malloc(sizeof(List));
    *list = NULL;
    return list;
}
// Q-2.2
Cell* buildCell(char* ch){
    Cell* cell = (Cell*)malloc(sizeof(Cell));
    cell->data = (char*)malloc(strlen(ch) + 1);
    strcpy(cell->data, ch);
    cell->next = NULL;
    return cell;
}
// Q-2.3
void insertFirst(List* L, Cell* C){
    if (*L ==NULL){
        *L = C;
    }else{
        C->next = *L;
        *L = C;
    }
}
// Q-2.4
char* ctos(Cell* C){
    return C->data;
}

char* ltos(List* L){
    while (*L != NULL){
        static char result[1024] = "";
        strcat(result, (*L)->data);
        strcat(result, "|");
        *L = (*L)->next;
        return result;
    }
}
// Q-2.5
Cell* listGet(List* L, int i){
    Cell* courant = *L;
    int j = 0;
    while (courant != NULL && j < i) {
        courant = courant->next;
        j++;
    }
    return courant;
}
// Q-2.6
Cell* searchList (List* L,char* ch){
    Cell* courant = *L;
    while (courant != NULL) {
        if (strcmp(courant->data, ch) == 0) {
            return courant;
        }
        courant = courant->next;
    }
    return NULL;
}
// Q-2.7
List* stol(char* s){
    List* L = initList();
    char* token = strtok(s, "|");
    while (token != NULL) {
        Cell* C = buildCell(token);
        insertFirst(L, C);
        token = strtok(NULL, "|");
    }
    return L;
}
// Q-2.8
void ltof(List* L, char* path){
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        perror("Erreur d'ouverture du fichier de destination");
        return;
    }
    Cell* courant = *L;
    while (courant != NULL) {
        fprintf(file, "%s\n", courant->data);
        courant = courant->next;
    }
    fclose(file);
}
List* ftol(char* path){
    List* L = initList();
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        perror("Erreur d'ouverture du fichier source");
        return L;
    }
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = 0; // Remove newline character
        Cell* C = buildCell(line);
        insertFirst(L, C);
    }
    fclose(file);
    return L;
}
// Q-3.1
List* listdir(char* root_dir){
    List* L = initList();
    DIR* dp = opendir(root_dir);
    if (dp == NULL) {
        perror("Erreur d'ouverture du répertoire");
        return L;
    }
    struct dirent* ep;
    while ((ep = readdir(dp)) != NULL)
    {
        if(strcmp(ep->d_name,".") == 0 || strcmp(ep->d_name,"..") == 0){
            continue;
        }
        insertFirst(L, buildCell(ep->d_name));
    }
    closedir(dp);
    return L;
}
// Q-3.2
int file_exists(char* file){
    List* L = listdir(".");
    Cell* courant = *L;
    while (courant != NULL) {
        if (strcmp(courant->data, file) == 0) {
            return 1; // File exists
        }
        courant = courant->next;
    }
    return 0;
}
// Q-3.3
void cp(char *to , char *from){
    FILE *src = fopen(from, "r");
    if (src == NULL){
        perror("Erreur d'ouverture du fichier source");
        return;
    }
    FILE *dest = fopen(to, "w");
    if (dest == NULL){
        perror("Erreur d'ouverture du fichier de destination");
        fclose(src);
        return;
    }
    char temp[256];
    while (fgets(temp, sizeof(temp), src) != NULL){
        fputs(temp, dest);
    }
    fclose(src);
    fclose(dest);
    printf("File copied from %s to %s\n", from, to);
}
// Q-3.4
char* hashToPath(char* hash){
    char* path[256];
    snprintf(path, sizeof(path), "%c%c/%s", hash[0], hash[1], hash + 2);
    return strdup(path);
}
// Q-3.5
void blobFile(char* file){
    char* hash = sha256file(file);
    if (hash == NULL) {
        printf("Error generating hash for file: %s\n", file);
        return;
    }
    char* path = hashToPath(hash);
    char dir[256];
    snprintf(dir, sizeof(dir), "%c%c", hash[0], hash[1]);
    char file[256];
    snprintf(file,sizeof(file), "%s", hash + 2);
    char command[512];
    snprintf(command, sizeof(command), "mkdir -p %s | touch %s/%s", dir, dir, file);
    int ret = system(command);
    if (ret == -1) {
        perror("Erreur lors de la création du répertoire");
        free(path);
        return;
    }
    cp(path, file);
    free(path);
}
// CHAPITRE 2 
// Enregistrement de plusieurs instantanés 
typedef struct {
    char* name;
    char* hash;
    int mode;
}WorkFile;
typedef struct {
    WorkFile* tab;
    int size;
    int n;
}WorkTree;

int getChmod(const char *path){
    struct stat ret;
    if (stat(path, &ret) == -1) {
        return -1;
    }
    return 
    (ret.st_mode & S_IRUSR)|(ret.st_mode & S_IWUSR)|(ret.st_mode & S_IXUSR)|/*owner*/
    (ret.st_mode & S_IRGRP)|(ret.st_mode & S_IWGRP)|(ret.st_mode & S_IXGRP)|/*group*/
    (ret.st_mode & S_IROTH)|(ret.st_mode & S_IWOTH)|(ret.st_mode & S_IXOTH);/*other*/

}
void setMode(int mode, char* path){
    char buff [100];
    sprintf(buff, "chmod %d %s", mode, path);
    system(buff);
}
// Q-4.1
WorkFile* createWorkFile(char* name){
    WorkFile* workFile = (WorkFile*)malloc(sizeof(WorkFile));
    workFile->name = strdup(name);
    workFile->hash = NULL;
    workFile->mode = 0;
    return workFile;
}
// Q-4.2
char* wfts(WorkFile* wf){
    static char result[1024] = "";
    snprintf(result, sizeof(result), "%s\t%s\t%d", wf->name, wf->hash, wf->mode);
    return result;
}
// Q-4.3
WorkFile* stwf(char*ch){
    WorkFile* wf = (WorkFile*)malloc(sizeof(WorkFile));
    char* token = strtok(ch, "\t");
    if (token != NULL) {
        wf->name = strdup(token);
        token = strtok(NULL, "\t");
    }
    if (token != NULL) {
        wf->hash = strdup(token);
        token = strtok(NULL, "\t");
    }
    if (token != NULL) {
        wf->mode = atoi(token);
    }
    return wf;
}
//Manipulation de worktree
// Q-4.4
WorkTree* initWorkTree(){
    const int WORKTREE_SIZE = 10; 
    WorkTree* workTree = (WorkTree*)malloc(sizeof(WorkTree));
    workTree->tab = (WorkFile*)malloc(WORKTREE_SIZE * sizeof(WorkFile));
    workTree->size = WORKTREE_SIZE;
    workTree->n = 0;
    return workTree;
}
// Q-4.5
int inWorkTree(WorkTree* wt, char* name){
    for (int i = 0; i < wt->n; i++) {
        if (strcmp(wt->tab[i].name, name) == 0) {
            return 1; // File exists in worktree
        }
    }
    return -1;
}
// Q-4.6
int appendWorkTree(WorkTree* wt,char* name,char* hash, int mode){
    if((wt->n == wt->size) && inWorkTree(wt, name) == -1){
        wt->size *= 2;
        wt->tab = (WorkFile*)realloc(wt->tab, wt->size * sizeof(WorkFile));
    }
    else if (inWorkTree(wt, name) != -1){
        return -1;
    }
    wt->tab[wt->n].hash = (char*)malloc(strlen(hash) + 1);
    strcpy(wt->tab[wt->n].hash, hash);
    wt->tab[wt->n].name = strdup(name);
    wt->tab[wt->n].mode = mode;
    wt->n++;
    return 0;
}
// Q-4.7
char* wtts(WorkTree* wt){
    static char result[1024] = "";
    for (int i = 0; i < wt->n; i++) {
        strcat(result, wfts(&wt->tab[i]));
        strcat(result, "\n");
    }
    return result;
}
// Q-4.8
WorkTree* stwt(char* ch){
    WorkTree* wt = initWorkTree();
    char* token = strtok(ch, "\n");
    while (token != NULL) {
        WorkFile* wf = stwf(token);
        appendWorkTree(wt, wf->name, wf->hash, wf->mode);
        token = strtok(NULL, "\n");
    }
    return wt;
}
// Q-4.9
int wttf(WorkTree* wt, char* file){
    FILE* f = fopen(file, "w");
    if (f == NULL) {
        perror("Erreur d'ouverture du fichier de destination");
        return -1;
    }
    fprintf(f, "%s", wtts(wt));
    fclose(f);
    return 0;
}
// Q-4.10
WorkTree* ftwt(char* file){
    WorkTree* wt = initWorkTree();
    FILE* f = fopen(file, "r");
    if (f == NULL) {
        perror("Erreur d'ouverture du fichier source");
        return wt;
    }
    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\n")] = 0;
        WorkFile* wf = stwf(line);
        appendWorkTree(wt, wf->name, wf->hash, wf->mode);
    }
    fclose(f);
    return wt;
}
// Enregistrement instantané et restauration d'un WorkTree
// Q-5.1
char* blobWorkTree(WorkTree* wt){
    char tempFileName[] = "/tmp/worktreeXXXXXX";
    int fd = mkstemp(tempFileName);
    if (fd == -1) {
        perror("Erreur de création du fichier temporaire");
        return NULL;
    }
    close(fd);

    if (wttf(wt, tempFileName) == -1) {
        unlink(tempFileName);
        return NULL;
    }

    char* hash = sha256file(tempFileName);
    if (hash == NULL) {
        unlink(tempFileName);
        return NULL;
    }

    char newFileName[512];
    snprintf(newFileName, sizeof(newFileName), "%s.t", hash);
    if (rename(tempFileName, newFileName) == -1) {
        perror("Erreur de renommage du fichier temporaire");
        unlink(tempFileName);
        return NULL;
    }

    return newFileName;
}
char* saveWorkTree(WorkTree* wt, char* path) {
    for (int i = 0; i < wt->n; i++) {
        char fullPath[512];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, wt->tab[i].name);

        struct stat st;
        if (stat(fullPath, &st) == -1) {
            perror("Erreur lors de l'accès au fichier ou répertoire");
            continue;
        }

        if (S_ISREG(st.st_mode)) { // If it's a regular file
            blobFile(fullPath);
            wt->tab[i].hash = sha256file(fullPath);
            wt->tab[i].mode = getChmod(fullPath);
        } else if (S_ISDIR(st.st_mode)) { // If it's a directory
            WorkTree* newWT = listdir(fullPath);
            char* dirHash = saveWorkTree(newWT, fullPath);
            wt->tab[i].hash = strdup(dirHash);
            wt->tab[i].mode = getChmod(fullPath);
            free(newWT->tab);
            free(newWT);
        }
    }
    return blobWorkTree(wt);
}
int main(){
    printf("Hash of file: %s\n", sha256file("../test.txt"));
}