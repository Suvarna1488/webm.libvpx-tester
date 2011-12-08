#include "vpxt_test_declarations.h"

int test_min_quantizer(int argc, const char *const *argv, const std::string &WorkingDir, std::string FilesAr[], int TestType, int DeleteIVF)
{
    char *CompressString = "Min Quantizer";
    char *MyDir = "test_min_quantizer";
    int inputCheck = vpxt_check_arg_input(argv[1], argc);

    if (inputCheck < 0)
        return vpxt_test_help(argv[1], 0);

    std::string input = argv[2];
    int Mode = atoi(argv[3]);
    int BitRate = atoi(argv[4]);
    std::string EncForm = argv[5];

    int speed = 0;

    ////////////Formatting Test Specific Directory////////////
    std::string CurTestDirStr = "";
    char MainTestDirChar[255] = "";
    std::string FileIndexStr = "";
    char FileIndexOutputChar[255] = "";

    if (initialize_test_directory(argc, argv, TestType, WorkingDir, MyDir, CurTestDirStr, FileIndexStr, MainTestDirChar, FileIndexOutputChar, FilesAr) == 11)
        return 11;

    std::string Min10QuantOutFile = CurTestDirStr;
    Min10QuantOutFile.append(slashCharStr());
    Min10QuantOutFile.append(MyDir);
    Min10QuantOutFile.append("_compression_10");
    vpxt_enc_format_append(Min10QuantOutFile, EncForm);

    std::string Min60QuantOutFile = CurTestDirStr;
    Min60QuantOutFile.append(slashCharStr());
    Min60QuantOutFile.append(MyDir);
    Min60QuantOutFile.append("_compression_60");
    vpxt_enc_format_append(Min60QuantOutFile, EncForm);

    /////////////OutPutfile////////////
    std::string TextfileString = CurTestDirStr;
    TextfileString.append(slashCharStr());
    TextfileString.append(MyDir);

    if (TestType == COMP_ONLY || TestType == TEST_AND_COMP)
        TextfileString.append(".txt");
    else
        TextfileString.append("_TestOnly.txt");

    FILE *fp;

    if ((fp = freopen(TextfileString.c_str(), "w", stderr)) == NULL)
    {
        tprintf(PRINT_STD, "Cannot open out put file: %s\n", TextfileString.c_str());
        exit(1);
    }

    ////////////////////////////////
    //////////////////////////////////////////////////////////

    if (TestType == TEST_AND_COMP)
        print_header_full_test(argc, argv, MainTestDirChar);

    if (TestType == COMP_ONLY)
        print_header_compression_only(argc, argv, MainTestDirChar);

    if (TestType == TEST_ONLY)
        print_header_test_only(argc, argv, CurTestDirStr);

    vpxt_cap_string_print(PRINT_BTH, "%s", MyDir);

    VP8_CONFIG opt;
    vpxt_default_parameters(opt);

    ///////////////////Use Custom Settings///////////////////
    if (inputCheck == 2)
    {
        if (!vpxt_file_exists_check(argv[argc-1]))
        {
            tprintf(PRINT_BTH, "\nInput Settings file %s does not exist\n", argv[argc-1]);

            fclose(fp);
            record_test_complete(FileIndexStr, FileIndexOutputChar, TestType);
            return 2;
        }

        opt = vpxt_input_settings(argv[argc-1]);
        BitRate = opt.target_bandwidth;
    }

    /////////////////////////////////////////////////////////
    double PSNRArr[2];
    opt.target_bandwidth = BitRate;
    opt.worst_allowed_q = 63;

    //make sure constrained quantizer mode is off
    if(opt.end_usage == 2)
        opt.end_usage = 1;

    //Run Test only (Runs Test, Sets up test to be run, or skips compresion of files)
    if (TestType == TEST_ONLY)
    {
        //This test requires no preperation before a Test Only Run
    }
    else
    {
        opt.Mode = Mode;

        opt.best_allowed_q = 10;

        if (vpxt_compress(input.c_str(), Min10QuantOutFile.c_str(), speed, BitRate, opt, CompressString, 10, 1, EncForm) == -1)
        {
            fclose(fp);
            record_test_complete(FileIndexStr, FileIndexOutputChar, TestType);
            return 2;
        }

        opt.best_allowed_q = 60;

        if (vpxt_compress(input.c_str(), Min60QuantOutFile.c_str(), speed, BitRate, opt, CompressString, 60, 1, EncForm) == -1)
        {
            fclose(fp);
            record_test_complete(FileIndexStr, FileIndexOutputChar, TestType);
            return 2;
        }
    }

    //Create Compression only stop test short.
    if (TestType == COMP_ONLY)
    {
        //Compression only run
        fclose(fp);
        record_test_complete(FileIndexStr, FileIndexOutputChar, TestType);
        return 10;
    }

    PSNRArr[0] = vpxt_psnr(input.c_str(), Min10QuantOutFile.c_str(), 0, 0, 1, NULL);
    PSNRArr[1] = vpxt_psnr(input.c_str(), Min60QuantOutFile.c_str(), 0, 0, 1, NULL);

    tprintf(PRINT_BTH, "\n");
    int Min10Q = vpxt_check_min_quantizer(Min10QuantOutFile.c_str(), 10);
    tprintf(PRINT_BTH, "\n\n");
    int Min60Q = vpxt_check_min_quantizer(Min60QuantOutFile.c_str(), 60);
    tprintf(PRINT_BTH, "\n\n");

    char Min10FileName[255] = "";
    char Min60FileName[255] = "";

    vpxt_file_name(Min10QuantOutFile.c_str(), Min10FileName, 0);
    vpxt_file_name(Min60QuantOutFile.c_str(), Min60FileName, 0);

    int fail = 0;

    tprintf(PRINT_BTH, "\n\nResults:\n\n");

    if (Min10Q != -1)
    {
        vpxt_formated_print(RESPRT, "Not all %s quantizers above MinQ - Failed", Min10FileName);
        tprintf(PRINT_BTH, "\n");
        fail = 1;
    }
    else
    {
        vpxt_formated_print(RESPRT, "All %s quantizers above MinQ - Passed", Min10FileName);
        tprintf(PRINT_BTH, "\n");
    }

    if (Min60Q != -1)
    {
        vpxt_formated_print(RESPRT, "Not all %s quantizers above MinQ - Failed", Min60FileName);
        tprintf(PRINT_BTH, "\n");
        fail = 1;
    }
    else
    {
        vpxt_formated_print(RESPRT, "All %s quantizers above MinQ - Passed", Min60FileName);
        tprintf(PRINT_BTH, "\n");
    }

    if (PSNRArr[0] <= PSNRArr[1])
    {
        vpxt_formated_print(RESPRT, "MinQ 10 PSNR: %2.2f <= MinQ 60 PSNR: %2.2f - Failed", PSNRArr[0], PSNRArr[1]);
        tprintf(PRINT_BTH, "\n");
        fail = 1;
    }
    else
    {
        vpxt_formated_print(RESPRT, "MinQ 10 PSNR: %2.2f > MinQ 60 PSNR: %2.2f - Passed", PSNRArr[0], PSNRArr[1]);
        tprintf(PRINT_BTH, "\n");
    }

    if (fail == 1)
    {
        tprintf(PRINT_BTH, "\nFailed\n");

        if (DeleteIVF)
            vpxt_delete_files(2, Min10QuantOutFile.c_str(), Min60QuantOutFile.c_str());

        fclose(fp);
        record_test_complete(FileIndexStr, FileIndexOutputChar, TestType);
        return 0;
    }
    else
    {
        tprintf(PRINT_BTH, "\nPassed\n");

        if (DeleteIVF)
            vpxt_delete_files(2, Min10QuantOutFile.c_str(), Min60QuantOutFile.c_str());

        fclose(fp);
        record_test_complete(FileIndexStr, FileIndexOutputChar, TestType);
        return 1;
    }

    fclose(fp);
    record_test_complete(FileIndexStr, FileIndexOutputChar, TestType);
    return 6;
}