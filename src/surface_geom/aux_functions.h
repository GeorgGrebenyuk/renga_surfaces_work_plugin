#pragma once
#include <string>
#include <vector>

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}
void del_from_list_by_edge(std::vector<std::vector<int>>* current_collection, std::vector<int>* what_del)
{
    //int where_is_element = 0;
    std::vector<std::vector<int>> new_list;
    for (std::vector<int> one_record : (*current_collection))
    {
        if ((one_record.at(0) == (*what_del).at(0) && one_record.at(1) == (*what_del).at(1)) |
            (one_record.at(0) == (*what_del).at(1) && one_record.at(1) == (*what_del).at(0)))
        {
            continue;
        }
        else {
            new_list.push_back(one_record);
        }
    }
    (*current_collection) = new_list;
}
bool is_element_in_list(std::vector<std::vector<int>>* current_collection, std::vector<int>* what_find)
{
    bool is_el = false;
    for (std::vector<int> one_record : (*current_collection))
    {
        if ((one_record.at(0) == (*what_find).at(0) && one_record.at(1) == (*what_find).at(1)) | 
            (one_record.at(0) == (*what_find).at(1) && one_record.at(1) == (*what_find).at(0)))
        {
            is_el = true;
            break;
        }
    }
    return is_el;
}
void insert_edge_if_not_in(std::vector<std::vector<int>>* current_collection, std::vector<int>* new_item)
{
    bool need_add = false;
    if ((*current_collection).empty()) need_add = true;
    else
    {
        std::vector<int> other_var{ new_item->at(1),new_item->at(0) };
        bool is_var_1 = is_element_in_list(current_collection, new_item);
        //bool is_var_2 = is_element_in_list(current_collection, &other_var);
        //���� � ������ ��� �� ������� �� ������� ��������� - �� ���������
        if (is_var_1 == false) need_add = true;
        //���� � ������ ���� ���� �� ���� ��������� (��������) �����, ������, ���������� �� ������� �� �������, � ����� �� �������
        else
        {
            need_add = false;
            del_from_list_by_edge(current_collection, new_item);
            //del_from_list_by_edge(current_collection, &other_var);
        }
    }
    if (need_add == true)
    {
        (*current_collection).push_back((*new_item));
    }

}
void start_loop(std::vector<std::vector<int>>* current_collection, std::vector<std::vector<int>>* outer_collection, int* point_find)
{
    for (std::vector<int> one_edge : (*current_collection))
    {
        if (one_edge.at(0) == (*point_find) | one_edge.at(1) == (*point_find))
        {
            (*outer_collection).push_back(one_edge);
            if (one_edge.at(0) == *point_find) *point_find = one_edge.at(1);
            else *point_find = one_edge.at(0);
            del_from_list_by_edge(current_collection, &one_edge);
            break;
        }

    }
}
void get_outer_counter(std::vector<std::vector<int>>* current_collection, std::vector<std::vector<int>>* outer_collection)
{
    std::cout << "current indexes" << std::endl;
    for (std::vector<int> one_record : (*current_collection))
    {
        std::cout << one_record.at(0) << " " << one_record.at(1) << std::endl;
    }
    //��������� ������ ������ - ���� �� "�������" ������, � ��������� �� �� ������� ������ � ����������� �� ������ - �������
    std::vector<int> start_edge = (*current_collection).at(0);
    del_from_list_by_edge(current_collection, &start_edge);
    (*outer_collection).push_back(start_edge);
    //��������� ������ �����, � ������� �� ������ ������ �� ���� ������ ���� ������� �����
    int end_index = start_edge.at(0);
    //��������� ������ �����, ������ �������� ����� ������ � ���������� ������ ���� ���������� ������� �����. ������� ��������
    int start_going = start_edge.at(1);
    int last_index = 0;

    do {
        start_loop(current_collection, outer_collection, &start_going);
    } while (start_going == end_index);
    //��������� ������� ������ ������� � �������: ���� � ������ ���-�� �������� - ������� ����� ����� �� ������� ��������� � ������� ������ 
    //- ���� � ���, ������ ��� ������� �������. � ��������� ������  - ���������� .... ���� �����, ���� ��� ������� "��������" ����� ����������� ����� 
    //� ����� ������� ���� ��� ����� ��� ������ ����������
}
